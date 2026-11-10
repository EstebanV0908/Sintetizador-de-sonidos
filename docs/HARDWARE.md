# Hardware y periféricos

Este documento registra únicamente conexiones y configuraciones verificables en `Generador_sonidos.ioc`, `main.c` y los archivos MSP. Los modelos de periféricos externos no presentes en el repositorio se dejan como no especificados.

## 1. Microcontrolador

- MCU: **STM32F407VET6**.
- Núcleo: Cortex-M4F.
- SYSCLK configurado: 168 MHz.
- HAL: STM32CubeF4.
- Paquete indicado por el `.ioc`: STM32Cube FW_F4 V1.28.3.

## 2. Tabla de conexiones

| Función | Componente externo | Pin STM32 | Recurso | Dirección / modo | Observación |
|---|---|---|---|---|---|
| Volumen | Potenciómetro, modelo no especificado | PA1 | ADC1_IN1 | Entrada analógica | ADC 12 bits, lectura por software. |
| Teclado fila 1 | Teclado 4×4 | PD0 | GPIO | Salida | Estado inicial alto. |
| Teclado fila 2 | Teclado 4×4 | PD1 | GPIO | Salida | Estado inicial alto. |
| Teclado fila 3 | Teclado 4×4 | PD2 | GPIO | Salida | Estado inicial alto. |
| Teclado fila 4 | Teclado 4×4 | PD3 | GPIO | Salida | Estado inicial alto. |
| Teclado columna 1 | Teclado 4×4 | PD4 | GPIO | Entrada + pull-up | Activo en bajo durante escaneo. |
| Teclado columna 2 | Teclado 4×4 | PD5 | GPIO | Entrada + pull-up | Activo en bajo durante escaneo. |
| Teclado columna 3 | Teclado 4×4 | PD6 | GPIO | Entrada + pull-up | Activo en bajo durante escaneo. |
| Teclado columna 4 | Teclado 4×4 | PD7 | GPIO | Entrada + pull-up | Activo en bajo durante escaneo. |
| Audio | Etapa de audio no especificada | PA4 | DAC_OUT1 | Salida analógica | DAC canal 1, trigger TIM6. |
| I²C SCL | Dispositivo no implementado | PB6 | I2C1_SCL | AF4 Open Drain | 400 kHz configurado. |
| I²C SDA | Dispositivo no implementado | PB7 | I2C1_SDA | AF4 Open Drain | 400 kHz configurado. |
| SWDIO | Depurador | PA13 | SWD | Debug | Según `.ioc`. |
| SWCLK | Depurador | PA14 | SWD | Debug | Según `.ioc`. |
| SWO | Depurador | PB3 | Trace SWO | Debug | Usado para `ITM_SendChar`. |

## 3. ADC

`ADC1` está configurado en `PA1`:

- resolución de 12 bits;
- alineación a la derecha;
- una conversión regular;
- inicio por software;
- modo continuo deshabilitado;
- sample time de 3 ciclos.

La aplicación convierte el rango 0–4095 a 0–100 % de volumen.

## 4. Teclado matricial

Las filas se mantienen en alto y se llevan individualmente a bajo. Las columnas usan pull-up interno, por lo que una tecla presionada produce lectura baja en la columna correspondiente cuando se activa su fila.

El periodo aproximado de escaneo es 20 ms, usado también como antirrebote básico.

## 5. DAC, TIM6 y DMA

### DAC

- Canal: DAC1 / OUT1.
- Pin: PA4.
- Trigger: TIM6 TRGO.
- Buffer de salida: habilitado.

### TIM6

- Prescaler: 0.
- Period: 2624.
- Master output trigger: Update.
- Frecuencia de actualización calculada: 32 kHz.

### DMA

- DMA1 Stream 5.
- Channel 7.
- Memory-to-peripheral.
- Incremento de memoria habilitado.
- Datos half-word.
- Modo circular.
- Prioridad DMA baja.

## 6. I²C

I2C1 está configurado a 400 kHz sobre PB6/PB7. Sin embargo, en el firmware de aplicación no aparecen transferencias `HAL_I2C_*()` para una pantalla u otro dispositivo. Por tanto, I²C se documenta como **configurado pero no funcionalmente utilizado por la aplicación actual**.

## 7. Consideraciones eléctricas

- No conectar un altavoz directamente al pin del microcontrolador si la carga excede la capacidad eléctrica permitida. Debe existir una etapa de acondicionamiento/amplificación apropiada.
- El potenciómetro debe entregar a PA1 una tensión dentro del rango analógico permitido por el STM32.
- Las señales GPIO deben respetar los niveles lógicos y tensiones máximas del STM32F407VET6.
- Si se utiliza I²C con módulos externos, revisar si el módulo incorpora resistencias pull-up. El MSP configura PB6/PB7 sin pull-up interno.

## 8. Información faltante para cerrar la documentación eléctrica

El repositorio no permite identificar con certeza:

- referencia y valor del potenciómetro;
- referencia del teclado matricial;
- circuito de amplificación o acondicionamiento de PA4;
- altavoz/buzzer empleado;
- pantalla OLED concreta, si existe físicamente;
- tensión de alimentación de cada módulo externo.

Estos datos deberían añadirse con base en el montaje físico real, no por suposición.
