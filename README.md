# SynthRTOS — Sintetizador de sonidos con STM32F407VET6

Sistema embebido que permite **tocar notas musicales con un teclado matricial de 4 × 4**, regular su volumen mediante un potenciómetro y consultar el estado de la reproducción en una **pantalla OLED SSD1306 por I²C**. La señal de audio se sintetiza con una tabla senoidal y se entrega por el DAC del STM32 mediante DMA circular.

Este repositorio corresponde al proyecto **Generador_sonidos** del reto de *Programación en Alto Nivel*. La documentación describe el firmware incluido en esta versión del ZIP; **no constituye una certificación de compilación ni de pruebas en hardware físico**.

## Objetivo y escenario de uso

El objetivo es construir un instrumento musical electrónico sencillo que integre lectura de entradas, generación digital de audio, visualización y tareas concurrentes con FreeRTOS. Está orientado a una demostración académica en la que una persona selecciona una nota mediante el teclado, varía el volumen y observa la información en la pantalla.

## Componentes y recursos utilizados

| Elemento | Función | Recurso STM32 |
|---|---|---|
| Placa basada en **STM32F407VET6** | Control y procesamiento | Cortex-M4 / HAL |
| Teclado matricial 4 × 4 | Selección de una de 16 notas | `PD0–PD3` (filas), `PD4–PD7` (columnas) |
| Potenciómetro | Control de volumen de 0 a 100 % | `PA1`, `ADC1_IN1` |
| Salida de audio y etapa externa apropiada | Reproducción de onda senoidal | `PA4`, `DAC_OUT1`, `TIM6`, DMA1 |
| Pantalla OLED compatible con **SSD1306** | Presentar nota, frecuencia, volumen o silencio | `PB6` (SCL), `PB7` (SDA), I²C1 |
| Depurador SWD | Programación y depuración | `PA13`, `PA14` |

## ¿Cómo funciona?

1. Al iniciar, el microcontrolador configura ADC, DAC, TIM6, DMA, GPIO e I²C. La pantalla se inicializa y presenta brevemente `Prueba OLED` antes de que la tarea de visualización escriba `SynthRTOS`.
2. `KeypadTask` lee el potenciómetro y escanea el teclado aproximadamente cada **20 ms más el tiempo de lectura/procesamiento**. Cuando cambia una nota o el volumen varía más de dos puntos, envía un mensaje a `keyQueue`.
3. `SynthTask` recibe la configuración, calcula el incremento de fase DDS y rellena por mitades el búfer de audio cuando el DMA lo notifica. `TIM6` establece **32 000 muestras por segundo**.
4. `SynthTask` remite los cambios a `displayQueue`. `DisplayTask` actualiza la OLED con nombre de nota, frecuencia en Hz y volumen en %, o `Silencio...` si se recibe frecuencia cero.
5. Al soltar el teclado, el audio se centra en el valor medio del DAC (`2048`). La pantalla depende de los mensajes recibidos; sin cambios de control, puede permanecer en la bienvenida inicial.

### Notas disponibles

| Fila del teclado | Columna 1 | Columna 2 | Columna 3 | Columna 4 |
|---|---|---|---|---|
| 1 | Do4 — 262 Hz | Do#4 — 277 Hz | Re4 — 294 Hz | Re#4 — 311 Hz |
| 2 | Mi4 — 330 Hz | Fa4 — 349 Hz | Fa#4 — 370 Hz | Sol4 — 392 Hz |
| 3 | Sol#4 — 415 Hz | La4 — 440 Hz | La#4 — 466 Hz | Si4 — 494 Hz |
| 4 | Do5 — 523 Hz | Do#5 — 554 Hz | Re5 — 587 Hz | Re#5 — 622 Hz |

## Arquitectura FreeRTOS

| Tarea | Responsabilidad | Prioridad CMSIS | Pila configurada | Activación |
|---|---|---|---|---|
| `SynthTask` | Generar muestras y coordinar DAC/DMA | `osPriorityHigh` | 2048 bytes | Notificación Half/Full del DMA (aprox. cada 2 ms) |
| `KeypadTask` | Leer ADC y teclado; producir eventos de control | `osPriorityNormal` | 1024 bytes | Bucle con `osDelay(20)` |
| `DisplayTask` | Actualizar OLED por I²C | `osPriorityLow` | 2048 bytes | Recepción de `displayQueue` |


## Estructura del repositorio

```text
Sintetizador-de-sonidos-main/
├── Core/
│   ├── Inc/                 # Cabeceras, configuración FreeRTOS y SSD1306
│   ├── Src/                 # main.c, freertos.c, driver OLED, ISR y HAL MSP
│   └── Startup/             # Arranque STM32F407
├── Drivers/                 # Bibliotecas CMSIS y HAL STM32F4
├── Middlewares/             # FreeRTOS y adaptador CMSIS-RTOS v2
├── Generador_sonidos.ioc    # Configuración STM32CubeMX
├── Generador_sonidos Debug.launch
├── STM32F407VETX_FLASH.ld
├── STM32F407VETX_RAM.ld
├── docs/                    # Documentación técnica y plan de validación
└── README.md
```

## Abrir y compilar en STM32CubeIDE

**Requisitos:** STM32CubeIDE, soporte STM32F4 (la configuración indica STM32Cube FW_F4 **V1.28.3**), ST-LINK o depurador compatible y la tarjeta física. En el ZIP recibido **no aparecen `.project` ni `.cproject`**, de modo que no es posible garantizar una importación directa como proyecto Eclipse/CubeIDE ya creado.

1. Guarda una copia del repositorio original.
2. Abre `Generador_sonidos.ioc` en STM32CubeIDE / CubeMX y verifica el microcontrolador **STM32F407VETx** y la opción de generación para **STM32CubeIDE**.
3. Si el IDE requiere generar los metadatos del proyecto, hazlo **en una copia separada**. La regeneración de código puede sobrescribir archivos, por lo que se deben conservar los archivos fuente suministrados y revisar los cambios antes de reemplazar nada.
4. Verifica que estén incluidos `Core/Src/ssd1306.c`, `Core/Src/ssd1306_fonts.c`, sus cabeceras y las bibliotecas HAL/FreeRTOS que aparecen en el ZIP.
5. Compila en configuración Debug o Release, conecta el hardware como en [HARDWARE.md](docs/HARDWARE.md), graba el firmware y ejecuta el [plan de pruebas](docs/PRUEBAS.md).

## Documentación técnica

- [Índice general](docs/INDEX.md)
- [Arquitectura y tareas FreeRTOS](docs/ARQUITECTURA.md)
- [Diagrama de bloques, conexiones y seguridad eléctrica](docs/HARDWARE.md)
- [Pruebas y formato para recoger evidencias](docs/PRUEBAS.md)

## Limitaciones actuales

- De principio se generaron varios problemas con la detección de la pantalla OLED 
