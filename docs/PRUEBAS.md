# Plan de pruebas y evidencias

El reto exige pruebas reproducibles y evidencia de funcionamiento.

## 1. Registro de pruebas

| ID | Prueba | Procedimiento | Resultado  |
|---|---|---|---|---|
| P01 | Arranque | Programar y energizar el STM32. | El scheduler inicia sin entrar en `Error_Handler`. |
| P02 | Reposo | No pulsar ninguna tecla. | `freq = 0`; DAC permanece en punto medio y `DisplayTask` puede reportar `I`. | 
| P03 | Teclado | Pulsar teclas de distintas filas/columnas. | Cambia la frecuencia de salida según `note_frequencies`. | 
| P04 | Volumen ADC | Mantener una nota y girar el potenciómetro. | Cambia la amplitud sin cambiar la frecuencia seleccionada. |
| P05 | Cambio de estado | Pulsar y liberar una tecla varias veces. | Reposo → reproducción → reposo sin reinicio. |
| P06 | Cola de control | Observar variables o trace mientras cambia nota/volumen. | `KeypadTask` produce mensajes y `SynthTask` los consume. |
| P07 | DMA/notificación | Observar callbacks Half/Full y `SynthTask`. | Se alternan las dos mitades del búfer sin espera activa. |
| P08 | Preempción | Ejecutar una tarea de menor prioridad y provocar callback DMA. | `SynthTask` queda Ready y se ejecuta con preferencia por su prioridad alta. |
| P09 | Estabilidad | Reproducir y cambiar notas/volumen durante ≥5 min. | Sin bloqueo, reset, corrupción audible persistente ni fallo del scheduler. |
| P10 | Sin tecla / pulsación inválida | Liberar todas las teclas o realizar pulsaciones rápidas. | Frecuencia vuelve a 0 y el sistema continúa operativo. |

## 2. Evidencia 


- foto del montaje físico completo;
- captura del `.ioc`;
- video del teclado cambiando notas;


## 3. Manejo de errores observado

-
-
-
