import json

import torch
from spikingjelly.activation_based import functional
from tqdm import tqdm

from .utils import normalize_targets


def _prepare_initial_state(targets):
    initial_state = targets[0]
    return initial_state.unsqueeze(-1) if initial_state.ndim == 1 else initial_state


def _supports_stateful_readout(model):
    return hasattr(model, "lif_out") and model.lif_out is not None


def _align_prediction_shape(predictions, targets):
    if predictions.ndim == targets.ndim + 1 and predictions.shape[-1] == 1:
        return predictions.squeeze(-1), targets
    return predictions, targets


def _sum_output_losses(loss_fn, predictions, targets):
    predictions, targets = _align_prediction_shape(predictions, targets)

    if predictions.ndim == targets.ndim == 2:
        return loss_fn(predictions, targets)

    if predictions.ndim == targets.ndim == 3:
        losses = [loss_fn(predictions[..., index], targets[..., index]) for index in range(predictions.shape[-1])]
        return torch.stack(losses).sum()

    raise ValueError(
        f"Unsupported prediction/target shapes for loss computation: {predictions.shape} vs {targets.shape}"
    )


def _mean_relative_error(predictions, targets):
    predictions, targets = _align_prediction_shape(predictions, targets)
    eps = torch.finfo(predictions.dtype).eps

    if predictions.ndim == targets.ndim == 2:
        denominator = torch.linalg.norm(targets).clamp_min(eps)
        return torch.linalg.norm(predictions - targets) / denominator

    if predictions.ndim == targets.ndim == 3:
        rel_errors = []
        for index in range(predictions.shape[-1]):
            denominator = torch.linalg.norm(targets[..., index]).clamp_min(eps)
            rel_error = torch.linalg.norm(predictions[..., index] - targets[..., index]) / denominator
            rel_errors.append(rel_error)
        return torch.stack(rel_errors).mean()

    raise ValueError(
        f"Unsupported prediction/target shapes for relative error computation: {predictions.shape} vs {targets.shape}"
    )


def validate(model, val_loader, CONFIG):
    """Validation loop - evaluates model without gradient computation."""
    device = CONFIG["device"]
    true_value_initialization = CONFIG["true_value_initialization"]
    transient = CONFIG["transient"]

    model.eval()

    loss_function_mse = torch.nn.MSELoss()
    loss_function_l1 = torch.nn.L1Loss()

    val_loss_mse_total = 0.0
    val_loss_l1_total = 0.0
    val_rel_err_total = 0.0
    iter_count = 0

    with torch.no_grad():
        pbar_val = tqdm(iter(val_loader), desc="  Validation", leave=False)

        for data, targets in pbar_val:
            iter_count += 1
            data = data.to(device)
            targets = normalize_targets(targets.to(device), CONFIG)

            num_steps = data.size(0)

            functional.reset_net(model)

            if true_value_initialization and _supports_stateful_readout(model):
                model.lif_out.v = _prepare_initial_state(targets)

            val_mem_list = []
            start_step = 1 if true_value_initialization else 0

            for step in range(start_step, num_steps):
                mem_out = model(data[step])
                val_mem_list.append(mem_out)

            batch_predictions = torch.stack(val_mem_list, dim=0)
            targets_aligned = targets[start_step:]

            t0 = max(0, transient - start_step)
            batch_predictions_eff = batch_predictions[t0:]
            targets_eff = targets_aligned[t0:]
            batch_predictions_eff, targets_eff = _align_prediction_shape(batch_predictions_eff, targets_eff)

            batch_loss_mse = _sum_output_losses(loss_function_mse, batch_predictions_eff, targets_eff)
            batch_loss_l1 = loss_function_l1(batch_predictions_eff, targets_eff)
            batch_rel_err = _mean_relative_error(batch_predictions_eff, targets_eff)

            val_loss_mse_total += batch_loss_mse.item()
            val_loss_l1_total += batch_loss_l1.item()
            val_rel_err_total += batch_rel_err.item()

            pbar_val.set_postfix({
                "mse": f"{batch_loss_mse.item():.6f}",
                "l1": f"{batch_loss_l1.item():.6f}",
            })

        pbar_val.close()

    avg_val_loss_mse = val_loss_mse_total / iter_count
    avg_val_loss_l1 = val_loss_l1_total / iter_count
    avg_val_rel_err = val_rel_err_total / iter_count

    return {
        "mse": avg_val_loss_mse,
        "l1": avg_val_loss_l1,
        "rel_err": avg_val_rel_err,
    }


def train(
    model,
    trainloader,
    valloader,
    CONFIG,
    output_dir,
    loss_fn=torch.nn.MSELoss(),
    use_wandb=False,
    project_name="snn-regression",
):
    with open(output_dir / "config.json", "w") as config_file:
        json.dump(CONFIG, config_file, indent=4)

    if use_wandb:
        import wandb

        run_name = (
            f"{CONFIG['block_type']}_transient_final_tau={CONFIG['final_tau']}_"
            f"tau={CONFIG['tau']}_norm={CONFIG['norm_type']}_plif={CONFIG['Plif']}"
        )

        wandb.init(
            project=project_name,
            name=run_name,
            config=CONFIG,
            dir=str(output_dir),
        )
        wandb.watch(model, log="all", log_freq=100)
        print("W&B initialized successfully")

    if CONFIG["optimizer"] == "SGD":
        optimizer = torch.optim.SGD(
            model.parameters(),
            lr=CONFIG["learning_rate"],
            momentum=CONFIG["momentum"],
            weight_decay=CONFIG["weight_decay"],
        )
    elif CONFIG["optimizer"] == "Adam":
        optimizer = torch.optim.Adam(
            model.parameters(),
            lr=CONFIG["learning_rate"],
            weight_decay=CONFIG["weight_decay"],
        )
    else:
        raise ValueError(f"Unknown optimizer: {CONFIG['optimizer']}")

    if CONFIG["scheduler"] == "ReduceLROnPlateau":
        scheduler = torch.optim.lr_scheduler.ReduceLROnPlateau(
            optimizer,
            mode="min",
            factor=CONFIG["scheduler_factor"],
            patience=CONFIG["scheduler_patience"],
            min_lr=CONFIG["min_lr"],
        )
    elif CONFIG["scheduler"] == "CosineAnnealing":
        scheduler = torch.optim.lr_scheduler.CosineAnnealingLR(
            optimizer,
            T_max=CONFIG["num_epochs"],
        )
    else:
        scheduler = None

    K = CONFIG["K"]
    true_value_initialization = CONFIG["true_value_initialization"]
    transient = CONFIG["transient"]
    num_epochs = CONFIG["num_epochs"]
    device = torch.device(CONFIG["device"])
    early_stop_patience = CONFIG["early_stop_patience"]

    print(f"\n{'=' * 70}")
    print("Starting TBPTT training")
    print(f"{'=' * 70}")
    print(f"  TBPTT window size (K): {K}")
    print(f"  Number of epochs: {num_epochs}")
    print(f"  Optimizer: {CONFIG['optimizer']}")
    print(f"  Learning rate: {CONFIG['learning_rate']}")
    print(f"  Device: {device}")
    print(f"{'=' * 70}\n")

    history = {
        "train_loss": [],
        "val_loss_mse": [],
        "val_loss_l1": [],
        "val_rel_err": [],
        "learning_rate": [],
        "best_epoch": 0,
    }

    best_val_loss = float("inf")
    best_epoch = 0
    patience_counter = 0

    for epoch in range(num_epochs):
        current_lr = optimizer.param_groups[0]["lr"]
        print(f"\n{'─' * 70}")
        print(f"Epoch {epoch + 1}/{num_epochs} | LR: {current_lr:.2e}")
        print(f"{'─' * 70}")

        model.train()
        epoch_loss = 0.0
        total_chunks = 0

        pbar_train = tqdm(iter(trainloader), desc="  Training", leave=True)

        for batch_idx, (data, targets) in enumerate(pbar_train):
            data = data.to(device)
            targets = normalize_targets(targets.to(device), CONFIG)
            num_steps = data.size(0)

            functional.reset_net(model)

            if true_value_initialization and _supports_stateful_readout(model):
                model.lif_out.v = _prepare_initial_state(targets)

            step_trunc = 0
            K_count = 0
            mem_rec_trunc = []
            batch_loss = 0.0
            batch_chunks = 0

            start_step = 1 if true_value_initialization else 0
            for step in range(start_step, num_steps):
                mem_out = model(data[step])
                mem_rec_trunc.append(mem_out)
                step_trunc += 1

                if step_trunc == K:
                    pred_chunk = torch.stack(mem_rec_trunc, dim=0)

                    start_idx = (K_count * K) + 1 if true_value_initialization else (K_count * K)
                    end_idx = start_idx + K
                    target_slice = targets[start_idx:end_idx]
                    loss = _sum_output_losses(loss_fn, pred_chunk, target_slice)

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

                    K_count += 1
                    step_trunc = 0
                    mem_rec_trunc = []

                if (step == num_steps - 1) and mem_rec_trunc:
                    pred_chunk = torch.stack(mem_rec_trunc, dim=0)
                    remaining_len = len(mem_rec_trunc)

                    if true_value_initialization:
                        start_idx = (K_count * K) + 1
                        end_idx = start_idx + remaining_len
                    else:
                        start_idx = K_count * K
                        end_idx = start_idx + remaining_len

                    target_slice = targets[int(start_idx):int(end_idx)]
                    loss = _sum_output_losses(loss_fn, pred_chunk, target_slice)

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

            avg_batch_loss = batch_loss / max(1, batch_chunks)
            pbar_train.set_postfix({"loss": f"{avg_batch_loss:.6f}"})

        pbar_train.close()

        avg_train_loss = epoch_loss / max(1, total_chunks)

        val_metrics = validate(model, valloader, CONFIG)
        avg_val_loss_mse = val_metrics["mse"]
        avg_val_loss_l1 = val_metrics["l1"]
        avg_val_rel_err = val_metrics["rel_err"]

        history["train_loss"].append(avg_train_loss)
        history["val_loss_mse"].append(avg_val_loss_mse)
        history["val_loss_l1"].append(avg_val_loss_l1)
        history["val_rel_err"].append(avg_val_rel_err)
        history["learning_rate"].append(current_lr)

        print(f"  Train Loss (MSE): {avg_train_loss:.6f}")
        print(f"  Val Loss (MSE):   {avg_val_loss_mse:.6f}")
        print(f"  Val Loss (L1):    {avg_val_loss_l1:.6f}")
        print(f"  Val Rel Error:    {avg_val_rel_err:.6f}")

        if use_wandb:
            wandb.log({
                "epoch/train_loss": avg_train_loss,
                "epoch/val_loss_mse": avg_val_loss_mse,
                "epoch/val_loss_l1": avg_val_loss_l1,
                "epoch/val_rel_err": avg_val_rel_err,
                "epoch/learning_rate": current_lr,
                "epoch/number": epoch + 1,
            })

        checkpoint_latest = {
            "epoch": epoch + 1,
            "model_state_dict": model.state_dict(),
            "optimizer_state_dict": optimizer.state_dict(),
            "scheduler_state_dict": scheduler.state_dict() if scheduler else None,
            "train_loss": avg_train_loss,
            "val_loss_mse": avg_val_loss_mse,
            "val_loss_l1": avg_val_loss_l1,
            "val_rel_err": avg_val_rel_err,
            "history": history,
            "config": CONFIG,
        }
        torch.save(checkpoint_latest, output_dir / "checkpoint_latest.pth")

        if avg_val_loss_mse < best_val_loss:
            best_val_loss = avg_val_loss_mse
            best_epoch = epoch + 1
            history["best_epoch"] = best_epoch
            patience_counter = 0

            torch.save(checkpoint_latest, output_dir / "checkpoint_best.pth")
            torch.save(model.state_dict(), output_dir / "best_model_weights.pth")

            print(f"  New best model saved. Val MSE: {best_val_loss:.6f}")

            if use_wandb:
                wandb.run.summary["best_val_loss_mse"] = best_val_loss
                wandb.run.summary["best_epoch"] = best_epoch
        else:
            patience_counter += 1
            print(f"  No improvement ({patience_counter}/{early_stop_patience})")

        if (epoch + 1) % 10 == 0:
            torch.save(checkpoint_latest, output_dir / f"checkpoint_epoch_{epoch + 1}.pth")
            print(f"  Periodic checkpoint saved (epoch {epoch + 1})")

        if scheduler:
            if isinstance(scheduler, torch.optim.lr_scheduler.ReduceLROnPlateau):
                scheduler.step(avg_val_loss_mse)
            else:
                scheduler.step()

        if patience_counter >= early_stop_patience:
            print(f"\n{'=' * 70}")
            print(f"Early stopping triggered. No improvement for {early_stop_patience} epochs")
            print(f"{'=' * 70}")
            break

    print(f"\n{'=' * 70}")
    print("Training completed")
    print(f"{'=' * 70}")
    print(f"  Best epoch: {best_epoch}")
    print(f"  Best val MSE loss: {best_val_loss:.6f}")
    print(f"  Final train loss: {history['train_loss'][-1]:.6f}")
    print(f"  Final val MSE loss: {history['val_loss_mse'][-1]:.6f}")
    print(f"  Final val L1 loss: {history['val_loss_l1'][-1]:.6f}")
    print(f"  Final val rel error: {history['val_rel_err'][-1]:.6f}")
    print(f"  Checkpoints saved in: {output_dir}")
    print(f"{'=' * 70}\n")

    with open(output_dir / "training_history.json", "w") as history_file:
        json.dump(history, history_file, indent=4)

    if use_wandb:
        artifact = wandb.Artifact("training_history", type="history")
        artifact.add_file(str(output_dir / "training_history.json"))
        wandb.log_artifact(artifact)
