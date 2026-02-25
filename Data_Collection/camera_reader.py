import time
import dv_processing as dv

# --- Abrir la cámara DAVIS ---
capture = dv.io.camera.open()

# Obtener resolución (solo hace falta una, DAVIS usa la misma para frames y eventos)
resolution = capture.getEventResolution()

# Crear configuración para DAVIS (frames + eventos + IMU + triggers)
config = dv.io.MonoCameraWriter.DAVISConfig("DAVIS346_sample", resolution)

# Crear el writer que guardará todo en un solo archivo .aedat4
writer = dv.io.MonoCameraWriter("output_davis346.aedat4", config)

print("Grabando frames y eventos... (Ctrl+C para detener)")

try:
    while capture.isRunning():
        # Leer eventos (batch)
        events = capture.getNextEventBatch()
        if events is not None:
            writer.writeEvents(events)

        # Leer frame (imagen)
        frame = capture.getNextFrame()
        if frame is not None:
            writer.writeFrame(frame)

        # Pequeña pausa para evitar uso excesivo de CPU
        #time.sleep(0.001)

except KeyboardInterrupt:
    print("\nGrabación detenida por el usuario.")

finally:
    # Cerrar correctamente la cámara y el writer
    del writer
    del capture
    print("Archivo guardado como output_davis346.aedat4")
