import os
import json
import torch
import numpy as np
from tqdm import tqdm
from spikingjelly.activation_based import functional

from .utils import normalize_targets

def validate(model, val_loader, CONFIG):
    """
    Validation loop - evaluates model without gradient computation.
    
    This function is called during training to assess model performance
    on unseen validation data. Unlike training, no weight updates occur.
    """
    
    device = CONFIG["device"]
    true_value_initialization = CONFIG["true_value_initialization"]
    transient = CONFIG["transient"]
    experiment_type = CONFIG["experiment"]  # Get experiment type
    
    model.eval()  # Set to evaluation mode

    # Loss functions
    loss_function_mse = torch.nn.MSELoss()
    loss_function_l1 = torch.nn.L1Loss()

    # Accumulators for metrics
    val_loss_mse_total = 0.0
    val_loss_l1_total = 0.0
    val_rel_err_total = 0.0
    iter_count = 0

    with torch.no_grad():  # Disable gradient computation
        pbar_val = tqdm(iter(val_loader), desc="  Validation", leave=False)

        for data, targets in pbar_val:
            iter_count += 1
            data = data.to(device)        # [T, B, C, H, W]
            targets = targets.to(device)  # [T, B]

            # Normalize targets to [-1, 1] range based on experiment type
            targets = normalize_targets(targets, experiment_type)

            num_steps = data.size(0)  # T (sequence length)

            # Reset all neuron states at the start of each sequence
            functional.reset_net(model)
            
            if true_value_initialization:
                model.lif_out.v = targets[0].unsqueeze(-1)  # Initialize output neuron state

            val_mem_list = []

            # When using true value initialization, skip first step used for init
            start_step = 1 if true_value_initialization else 0

            # Process entire validation sequence
            for step in range(start_step, num_steps):
                mem_out = model(data[step])  # Forward pass: [B, 1]
                val_mem_list.append(mem_out)

            # Stack all predictions: [T', B, 1] → [T', B]
            batch_predictions = torch.stack(val_mem_list, dim=0)
            batch_predictions = batch_predictions.squeeze(-1)

            # Align targets with prediction start when init is used
            targets_aligned = targets[start_step:]

            # Skip transient period (warmup), adjusted for start_step
            t0 = max(0, transient - start_step)
            batch_predictions_eff = batch_predictions[t0:]
            targets_eff = targets_aligned[t0:]

            # Compute metrics for this batch
            batch_loss_mse = loss_function_mse(batch_predictions_eff, targets_eff)
            batch_loss_l1 = loss_function_l1(batch_predictions_eff, targets_eff)
            batch_rel_err = torch.linalg.norm(batch_predictions_eff - targets_eff) / torch.linalg.norm(targets_eff)

            val_loss_mse_total += batch_loss_mse.item()
            val_loss_l1_total += batch_loss_l1.item()
            val_rel_err_total += batch_rel_err.item()

            # Update progress bar
            pbar_val.set_postfix({
                'mse': f'{batch_loss_mse.item():.6f}',
                'l1': f'{batch_loss_l1.item():.6f}'
            })

        pbar_val.close()

    # Compute average metrics across all validation batches
    avg_val_loss_mse = val_loss_mse_total / iter_count
    avg_val_loss_l1 = val_loss_l1_total / iter_count
    avg_val_rel_err = val_rel_err_total / iter_count

    return {
        'mse': avg_val_loss_mse,
        'l1': avg_val_loss_l1,
        'rel_err': avg_val_rel_err
    }


def train(model, trainloader, valloader, CONFIG, output_dir, loss_fn=torch.nn.MSELoss(), use_wandb=False, project_name="snn-regression"):
    # ============================================================================
    # Create output directory for checkpoints and logs
    # ============================================================================

    # Save configuration to JSON file
    with open(output_dir / "config.json", "w") as f:
        json.dump(CONFIG, f, indent=4)

    # ============================================================================
    # Initialize Weights & Biases
    # ============================================================================
    if use_wandb:
        import wandb

        # Automatic run name:
        run_name = f"{CONFIG['block_type']}_transient_final_tau={CONFIG['final_tau']}_tau={CONFIG['tau']}_norm={CONFIG['norm_type']}_plif={CONFIG['Plif']}"

        wandb.init(
            project=project_name,
            name=run_name,
            config=CONFIG,
            dir=str(output_dir)  # Save wandb files in output_dir
        )
        # Watch model for gradient and parameter tracking
        wandb.watch(model, log="all", log_freq=100)
        print("W&B initialized successfully")
        
    # ============================================================================
    # Optimizer Setup
    # ============================================================================

    if CONFIG["optimizer"] == "SGD":
        optimizer = torch.optim.SGD(
            model.parameters(), 
            lr=CONFIG["learning_rate"], 
            momentum=CONFIG["momentum"],
            weight_decay=CONFIG["weight_decay"]
        )
    elif CONFIG["optimizer"] == "Adam":
        optimizer = torch.optim.Adam(
            model.parameters(), 
            lr=CONFIG["learning_rate"],
            weight_decay=CONFIG["weight_decay"]
        )
    else:
        raise ValueError(f"Unknown optimizer: {CONFIG['optimizer']}")

    # ============================================================================
    # Learning Rate Scheduler
    # ============================================================================

    if CONFIG["scheduler"] == "ReduceLROnPlateau":
        # Reduce LR when validation loss plateaus
        scheduler = torch.optim.lr_scheduler.ReduceLROnPlateau(
            optimizer, 
            mode='min',
            factor=CONFIG["scheduler_factor"],
            patience=CONFIG["scheduler_patience"],
            min_lr=CONFIG["min_lr"]
        )
    elif CONFIG["scheduler"] == "CosineAnnealing":
        # Cosine annealing schedule
        scheduler = torch.optim.lr_scheduler.CosineAnnealingLR(
            optimizer, 
            T_max=CONFIG["num_epochs"]
        )
    else:
        scheduler = None

    # Training parameters
    K = CONFIG["K"]  # TBPTT window size
    true_value_initialization = CONFIG["true_value_initialization"]
    transient = CONFIG["transient"]
    num_epochs = CONFIG["num_epochs"]
    device = torch.device(CONFIG["device"])
    early_stop_patience = CONFIG["early_stop_patience"]
    experiment_type = CONFIG["experiment"]  # Get experiment type

    print(f"\n{'='*70}")
    print("Starting TBPTT training")
    print(f"{'='*70}")
    print(f"  TBPTT window size (K): {K}")
    print(f"  Number of epochs: {num_epochs}")
    print(f"  Optimizer: {CONFIG['optimizer']}")
    print(f"  Learning rate: {CONFIG['learning_rate']}")
    print(f"  Device: {device}")
    print(f"{'='*70}\n")

    # History tracking
    history = {
        'train_loss': [],
        'val_loss_mse': [],
        'val_loss_l1': [],
        'val_rel_err': [],
        'learning_rate': [],
        'best_epoch': 0
    }

    # Best model tracking
    best_val_loss = float('inf')
    best_epoch = 0
    patience_counter = 0

    # ============================================================================
    # MAIN TRAINING LOOP
    # ============================================================================

    for epoch in range(num_epochs):
        current_lr = optimizer.param_groups[0]['lr']
        print(f"\n{'─'*70}")
        print(f"Epoch {epoch+1}/{num_epochs} | LR: {current_lr:.2e}")
        print(f"{'─'*70}")
        
        # ========================================================================
        # TRAINING PHASE
        # ========================================================================
        model.train()  # Training mode
        epoch_loss = 0.0
        total_chunks = 0
        
        pbar_train = tqdm(iter(trainloader), desc=f"  Training", leave=True)
        
        for batch_idx, (data, targets) in enumerate(pbar_train):
            data = data.to(device)
            targets = targets.to(device)
            targets = normalize_targets(targets, experiment_type)  # Normalize based on experiment
            
            num_steps = data.size(0)
            
            # Reset hidden states at start of sequence
            functional.reset_net(model)
            
            if true_value_initialization:
                model.lif_out.v = targets[0].unsqueeze(-1)  # Initialize output neuron state
            
            step_trunc = 0
            K_count = 0
            mem_rec_trunc = []
            batch_loss = 0.0
            batch_chunks = 0
            
            # Process through entire sequence with TBPTT
            # When true_value_initialization=True, start from step 1 (skip the first step used for initialization)
            start_step = 1 if true_value_initialization else 0
            for step in range(start_step, num_steps):
                mem_out = model(data[step])
                mem_rec_trunc.append(mem_out)
                step_trunc += 1

                # Backward pass every K steps
                if step_trunc == K:
                    mem_rec_trunc = torch.stack(mem_rec_trunc, dim=0)
                    
                    # Adjust indices when using true_value_initialization
                    if true_value_initialization:
                        start_idx = int(K_count * K) + 1
                    else:
                        start_idx = int(K_count * K)
                        
                    end_idx = start_idx + K    
                    target_slice = targets[start_idx:end_idx]
                    loss = loss_fn(mem_rec_trunc.squeeze(-1), target_slice)

                    # Backward and optimize
                    optimizer.zero_grad()
                    loss.backward()
                    optimizer.step()
                    model.detach()  # Truncate gradients

                    if step >= transient:
                        epoch_loss += loss.item()
                        batch_loss += loss.item()
                        total_chunks += 1
                        batch_chunks += 1

                        if use_wandb:
                            wandb.log({
                                "train/chunk_loss": loss.item(),
                                "train/learning_rate": current_lr,
                                "train/epoch": epoch + 1,
                                "train/batch": batch_idx,
                            })

                    # Reset
                    K_count += 1
                    step_trunc = 0
                    mem_rec_trunc = []
                
                # Handle remaining timesteps
                if (step == num_steps - 1) and (mem_rec_trunc):
                    mem_rec_trunc = torch.stack(mem_rec_trunc, dim=0)
                    
                    # Adjust indices for remaining timesteps when using true_value_initialization
                    if true_value_initialization:
                        remaining_len = len(mem_rec_trunc)
                        start_idx = (K_count * K) + 1
                        end_idx = start_idx + remaining_len
                    else:
                        start_idx = K_count * K
                        end_idx = K_count * K + num_steps % K
                        
                    target_slice = targets[int(start_idx):int(end_idx)]
                    loss = loss_fn(mem_rec_trunc.squeeze(-1), target_slice)

                    optimizer.zero_grad()
                    loss.backward()
                    optimizer.step()
                    model.detach()

                    if step >= transient:
                        epoch_loss += loss.item()
                        batch_loss += loss.item()
                        total_chunks += 1
                        batch_chunks += 1

                        if use_wandb:
                            wandb.log({
                                "train/chunk_loss": loss.item(),
                                "train/learning_rate": current_lr,
                                "train/epoch": epoch + 1,
                                "train/batch": batch_idx,
                            })
                
            # Update progress bar
            avg_batch_loss = batch_loss / max(1, batch_chunks)
            pbar_train.set_postfix({'loss': f'{avg_batch_loss:.6f}'})
        
        pbar_train.close()
        
        # ========================================================================
        # COMPUTE EPOCH METRICS
        # ========================================================================
        avg_train_loss = epoch_loss / max(1, total_chunks)
        
        # ========================================================================
        # VALIDATION PHASE
        # ========================================================================
        val_metrics = validate(model, valloader, CONFIG)
        avg_val_loss_mse = val_metrics['mse']
        avg_val_loss_l1 = val_metrics['l1']
        avg_val_rel_err = val_metrics['rel_err']
        
        # ========================================================================
        # UPDATE HISTORY
        # ========================================================================
        history['train_loss'].append(avg_train_loss)
        history['val_loss_mse'].append(avg_val_loss_mse)
        history['val_loss_l1'].append(avg_val_loss_l1)
        history['val_rel_err'].append(avg_val_rel_err)
        history['learning_rate'].append(current_lr)
        
        # ========================================================================
        # PRINT EPOCH SUMMARY
        # ========================================================================
        print(f"  Train Loss (MSE): {avg_train_loss:.6f}")
        print(f"  Val Loss (MSE):   {avg_val_loss_mse:.6f}")
        print(f"  Val Loss (L1):    {avg_val_loss_l1:.6f}")
        print(f"  Val Rel Error:    {avg_val_rel_err:.6f}")
        
        # ========================================================================
        # LOG TO WANDB
        # ========================================================================
        if use_wandb:
            wandb.log({
                "epoch/train_loss": avg_train_loss,
                "epoch/val_loss_mse": avg_val_loss_mse,
                "epoch/val_loss_l1": avg_val_loss_l1,
                "epoch/val_rel_err": avg_val_rel_err,
                "epoch/learning_rate": current_lr,
                "epoch/number": epoch + 1,
            })
        # ========================================================================
        # SAVE CHECKPOINTS
        # ========================================================================
        
        # 1. ALWAYS save latest checkpoint (to resume training)
        checkpoint_latest = {
            'epoch': epoch + 1,
            'model_state_dict': model.state_dict(),
            'optimizer_state_dict': optimizer.state_dict(),
            'scheduler_state_dict': scheduler.state_dict() if scheduler else None,
            'train_loss': avg_train_loss,
            'val_loss_mse': avg_val_loss_mse,
            'val_loss_l1': avg_val_loss_l1,
            'val_rel_err': avg_val_rel_err,
            'history': history,
            'config': CONFIG,
        }
        torch.save(checkpoint_latest, output_dir / "checkpoint_latest.pth")
        
        # 2. Save BEST model (based on validation MSE loss - same as training)
        if avg_val_loss_mse < best_val_loss:
            best_val_loss = avg_val_loss_mse
            best_epoch = epoch + 1
            history['best_epoch'] = best_epoch
            patience_counter = 0  # Reset patience
            
            torch.save(checkpoint_latest, output_dir / "checkpoint_best.pth")
            # Also save just the model weights (lighter file)
            torch.save(model.state_dict(), output_dir / "best_model_weights.pth")
            
            print(f"  New best model saved. Val MSE: {best_val_loss:.6f}")
            
            if use_wandb:
                wandb.run.summary["best_val_loss_mse"] = best_val_loss
                wandb.run.summary["best_epoch"] = best_epoch
        else:
            patience_counter += 1
            print(f"  No improvement ({patience_counter}/{early_stop_patience})")
        
        # 3. Save periodic checkpoints (every 10 epochs)
        if (epoch + 1) % 10 == 0:
            torch.save(checkpoint_latest, output_dir / f"checkpoint_epoch_{epoch+1}.pth")
            print(f"  Periodic checkpoint saved (epoch {epoch+1})")
        
        # ========================================================================
        # LEARNING RATE SCHEDULING
        # ========================================================================
        if scheduler:
            if isinstance(scheduler, torch.optim.lr_scheduler.ReduceLROnPlateau):
                scheduler.step(avg_val_loss_mse)  # Use validation MSE loss
            else:
                scheduler.step()  # Use epoch number
        
        # ========================================================================
        # EARLY STOPPING CHECK
        # ========================================================================
        if patience_counter >= early_stop_patience:
            print(f"\n{'='*70}")
            print(f"Early stopping triggered. No improvement for {early_stop_patience} epochs")
            print(f"{'='*70}")
            break
    # ============================================================================
    # TRAINING FINISHED
    # ============================================================================

    print(f"\n{'='*70}")
    print("Training completed")
    print(f"{'='*70}")
    print(f"  Best epoch: {best_epoch}")
    print(f"  Best val MSE loss: {best_val_loss:.6f}")
    print(f"  Final train loss: {history['train_loss'][-1]:.6f}")
    print(f"  Final val MSE loss: {history['val_loss_mse'][-1]:.6f}")
    print(f"  Final val L1 loss: {history['val_loss_l1'][-1]:.6f}")
    print(f"  Final val rel error: {history['val_rel_err'][-1]:.6f}")
    print(f"  Checkpoints saved in: {output_dir}")
    print(f"{'='*70}\n")

    # Save final history
    with open(output_dir / "training_history.json", "w") as f:
        json.dump(history, f, indent=4)

    if use_wandb:
        # Save history as artifact
        artifact = wandb.Artifact('training_history', type='history')
        artifact.add_file(str(output_dir / "training_history.json"))
        wandb.log_artifact(artifact)