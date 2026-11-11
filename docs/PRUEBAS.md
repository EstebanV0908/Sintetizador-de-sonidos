# Plan de pruebas y registro de evidencias


## 1. Condiciones iniciales

- MCU STM32F407VET6 programado con el firmware correspondiente a este ZIP.
- Teclado 4 × 4 conectado a `PD0–PD7` y potenciómetro a `PA1`.
- Pantalla OLED compatible con el controlador SSD1306 conectada a `PB6/PB7` con alimentación adecuada.
- Etapa de audio de entrada compatible con DAC conectada a `PA4` y salida de escucha/medición segura.
- Conexión de depuración ST-LINK si se recopila evidencia de RTOS, DMA y memoria.
- Registrar fecha, placa, tensión de alimentación, versión de IDE y responsable de la prueba.

## 2. Matriz de pruebas reproducibles

| ID | Caso | Procedimiento | Resultado esperado del firmware |
|---|---|---|---|
| P01 | Inicialización | Encender o resetear el equipo. | Inicializa periféricos y OLED; se envía `Prueba OLED`, luego `SynthRTOS`. |
| P02 | Silencio | Soltar todas las teclas después de haber tocado una nota. | `freq=0`, señal DAC centrada en 2048; OLED muestra `Silencio...` tras el mensaje. |
| P03 | 16 teclas | Presionar individualmente cada tecla de la matriz. | La frecuencia corresponde a `note_frequencies[4][4]`; la OLED identifica nota y Hz. |
| P04 | Volumen | Mantener pulsada una nota y girar el potenciómetro. | Cambia la amplitud del DAC y el valor `Vol: ... %` cuando se supera el umbral de cambio. |
| P05 | Cambio sin reset | Alternar tecla presionada/liberada varias veces. | Cambia entre reproducción y silencio sin reiniciar MCU. |
| P06 | Cola de teclas | Depurar `keyQueue` mientras cambian nota y volumen. | `KeypadTask` envía `ControlMsg_t` y `SynthTask` lo recibe. |
| P07 | Cola de pantalla | Depurar `displayQueue` y observar OLED durante cambios. | `SynthTask` envía nuevos datos a `DisplayTask`, que refresca la pantalla. |
| P08 | DMA y síntesis | Medir callbacks DMA y actividad de `SynthTask`. | Notificaciones alternan mitades de 64 muestras a ≈2 ms cada una. |
| P09 | Preempción | Registrar línea de tiempo de una tarea menos prioritaria y el despertar de `SynthTask`. | Si es la tarea lista de mayor prioridad, `SynthTask` ejecuta antes de reanudar tarea inferior. |
| P10 | Estabilidad ≥5 min | Reproducir, soltar teclas, cambiar notas y volumen durante 5 minutos continuos. | Sin reset, suspensión inesperada ni errores de pantalla/audio persistentes. |
| P11 | Límites de ADC | Llevar el potenciómetro a ambos extremos del rango válido. | Volumen cercano a 0 % y 100 % sin valores fuera de rango. |
| P12 | Entrada inesperada | Hacer pulsaciones rápidas, liberar y pulsar varias teclas. | El firmware continúa operando; no produce polifonía, solo la primera tecla detectada. |
| P13 | Falla periférica | Simular una pantalla no conectada **solo en una prueba controlada y segura**. | Verificar si la comunicación se bloquea; el código no maneja explícitamente errores I²C. |
| P14 | Margen de pila | Medir el mínimo libre de pila de cada tarea en ejecución real. | Registrar el margen obtenido; no existe valor verificado en el repositorio. |
| P15 | Compilación / importación | Abrir proyecto o reconstruir metadatos a partir del `.ioc` en copia; compilar. | Comprobar warnings y errores reales; no se adjuntan `.project`/`.cproject`. |

## 3. Evidencias recomendadas para adjuntar al repositorio

-fotitos 
