# est-climatica
Estación climática simple basada en un esp8266 y varios sensores

# TEMT6000: sensor de luz
Se iba a utilizar en un principio pero luego su rango de funcionamiento resultaba insuficiente. Sólo podía dar valores de hasta 1000 lx. Tengo un documento más explicativo [aquí (Github)](https://github.com/Xayiide/esp-8266-pruebas/blob/main/arduino/temt6000/README.md).

# AM2315C: sensor de temperatura y humedad 
Se utiliza para medir temperatura y humedad. Una librería para Arduino se encuentra [en Github](https://github.com/RobTillaart/AM2315C), y la documentación en la [página de Adafruit](https://cdn-shop.adafruit.com/product-files/5182/5182_AM2315C.pdf).

# VEML7700: sensor de luz
- [Documentación del sensor de Vishay](https://www.vishay.com/docs/84286/veml7700.pdf).
- [Vishay: Diseñando aplicaciones con el VEML7700](https://www.vishay.com/docs/84323/designingveml7700.pdf).
- [Documentación del módulo de Adafruit](https://cdn-learn.adafruit.com/downloads/pdf/adafruit-veml7700.pdf).
- [Guía de Adafruit con Arduino](https://learn.adafruit.com/adafruit-veml7700/arduino).
- [Repositorio de la librería para Arduino](https://github.com/adafruit/Adafruit_VEML7700).

Para hacer el componente dedicado a este sensor, me he ayudado inmensamente del [repositorio](https://github.com/kgrozdanovski/veml7700-esp-idf) del usuario [Kgrozdanovski](https://github.com/kgrozdanovski/), que hace una librería de este sensor usando el SDK de Espressif.

## Ganancia
En este sensor mencionan la ganancia en varios lados, porque es configurable. La ganancia es una magnitud que mide cuánto se amplifica o se disminuye una señal (el ruido es tb una señal). Cuando el sensor mida valores muy bajos de luz, interesará una ganancia mayor, porque de esta forma obtendremos más precisión en las medidas. El caso contrario también aplica: con luz más intensa, una menor ganancia.

## Tiempo de integración
Indica durante cuánto tiempo se "construye" la señal. Si se necesita mayor resolución, se puede aumentar el tiempo de integración. Si se prefieren lecturas rápidas, podrá reducirse.
