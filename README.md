# OnControl ESP32 / Wokwi

Simulador de la banda IoT de OnControl. Envia BPM, SpO2 y temperatura al
EdgeService desplegado y permite ver la lectura en el frontend.

## Flujo

```text
Wokwi / ESP32 -> EdgeService en Render -> Frontend en Vercel
```

## Ejecutar en Wokwi

1. Importar este repositorio desde Wokwi.
2. Iniciar la simulacion.
3. Esperar en el monitor serial la respuesta HTTP `201`.
4. Ajustar los potenciometros para cambiar BPM, SpO2 y temperatura.
5. Pulsar el boton **ENVIAR** para registrar otra lectura.
6. Abrir `https://oncontrol-frontend.vercel.app/demo-iot` o el monitor IoT del
   dashboard para comprobar la lectura.

El simulador envia una sola medicion al arrancar. Las siguientes mediciones se
envian unicamente al pulsar el boton, evitando llamadas innecesarias.

## Proyecto para sensores fisicos

La implementacion original para los sensores MAX30102 y MLX90614 se conserva
en [`hardware/`](hardware/). Antes de cargarla en un dispositivo fisico se
deben configurar la red WiFi y la URL publica del EdgeService.
