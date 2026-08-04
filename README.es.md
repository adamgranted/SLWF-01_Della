<!-- Traducción de README.md. Mantener sincronizados los modelos, funciones y pasos de instalación. -->
<!-- esphome-della-ac — control local en Home Assistant para aires acondicionados mini split Della -->

<div align="center">
  <a href="https://github.com/adamgranted/esphome-della-ac">
    <picture>
      <source srcset="./.github/img/della-logo-dark.svg" media="(prefers-color-scheme: dark)">
      <img src="./.github/img/della-logo-light.svg" alt="Della" height="44"/>
    </picture>
  </a>
  <h2>esphome-della-ac</h2>
  <p>
    <a href="./README.md">English</a> · <b>Español</b>
  </p>
  <p align="center">
      <p><b>Control local en Home Assistant para mini splits Della fabricados por AUX</b></p>
  </p>

  <p align="center">
    <img alt="platform" src="https://img.shields.io/badge/platform-ESP8266-blue">
    <img alt="esphome" src="https://img.shields.io/badge/ESPHome-2026.5%2B-1c1c1c">
    <img alt="protocol" src="https://img.shields.io/badge/protocol-AUX%204800%208E1-orange">
  </p>

</div>


<br>

Firmware para ESPHome que integra un mini split **Della fabricado por AUX** como un termostato completo en Home Assistant: sin nube ni cuenta de Tuya. Sustituye el módulo Wi-Fi de fábrica por un [SMLIGHT SLWF-01](https://smlight.tech/) con este firmware, listo para conectar al puerto de servicio USB-A de Della. Se desarrolló con un **Della 048-MS**; consulte [Unidades compatibles](#unidades-compatibles) para ver la lista de modelos.

Estas unidades **no** utilizan el protocolo TCL que parece sugerir su puerto USB. Son equipos **OEM de AUX** y se comunican mediante el protocolo serie AUX HVAC a **4800 baudios, 8E1**, sobre una línea de drenador abierto (open-drain). Muestrear esa línea a 9600 baudios genera una secuencia de bytes convincente, pero falsa; esa trampa de aliasing explica por qué los intentos anteriores no funcionaron. La explicación completa y el mapa de bytes se encuentran en [`docs/PROTOCOL.md`](docs/PROTOCOL.md).


## Unidades compatibles

Un mismo firmware sirve para todos los modelos: identifica automáticamente la variante de la trama de estado del aire acondicionado durante la ejecución. Por tanto, hay **una sola compilación para todos los modelos** y no es necesario elegir una imagen distinta para cada uno.

| Modelo | Control | Telemetría | Notas |
|-------|:------:|:--------:|-------|
| **Della 048-MS** | ✅ | ✅ | Unidad de referencia, completamente verificada |
| **Della Motto JA 12K** (`12K1VRH-20S-JA`) | ✅ | ✅ | Utiliza el mismo protocolo AUX que el 048-MS (trama de estado de 35 bytes). Confirmado mediante capturas en reposo y durante refrigeración activa ([#11](https://github.com/adamgranted/esphome-della-ac/issues/11)) |
| **Della Serena Series 18K** (`18K2VR-22S-M-I+O`) | ✅ | ✅ | Un usuario de la comunidad confirmó su funcionamiento ([informe en la comunidad de Home Assistant](https://community.home-assistant.io/t/della-ac-integration/756819/16)) |

Es muy probable que funcionen otros modelos Della fabricados por AUX. Si el suyo no figura en la lista, abra una incidencia e incluya una captura del registro con `verbose` activado; normalmente se puede añadir con una o dos líneas de código.


## Características

- **Entidad de climatización completa** — off / cool / heat / dry / fan-only / heat_cool; ventilador automático / bajo / medio / alto / silencioso; oscilación vertical / horizontal / ambas; modos boost (turbo) y sleep (sueño), temperatura actual y acción HVAC
- **Controles de funciones** — entidades de Home Assistant para la pantalla del panel, el ionizador, el modo eco, la autolimpieza y el ciclo de secado antihongos. Cada control reproduce el comportamiento real del mando a distancia; las funciones que solo actúan con la unidad apagada se gestionan como tales, en vez de mostrarse como interruptores sin efecto
- **Sensores de telemetría y estado** — temperaturas del evaporador, del compresor y del exterior; porcentaje de potencia del inversor; velocidad real del ventilador; temperatura objetivo y un resumen del estado en una sola línea legible
- **Recepción fiable (RX)** — captura pulsos de forma inmune a los flancos de subida irregulares del MCU del aire acondicionado; después reconstruye cada trama y comprueba su CRC en el dispositivo
- **Escrituras seguras** — cada comando se construye copiando la trama de estado más reciente y modificando únicamente los campos solicitados; se rechaza si la última lectura ya no es reciente
- **Estado a 1 Hz** — actualizaciones en menos de 2 segundos en Home Assistant; la carga del bus se mantiene ligera
- **Funcionamiento local** — API nativa de ESPHome + OTA; nada sale de su red


## Hardware

- Un mini split **Della fabricado por AUX** — consulte [Unidades compatibles](#unidades-compatibles) para ver los modelos confirmados. Es muy probable que también funcionen otros modelos Della fabricados por AUX.
- Un **SMLIGHT SLWF-01** (ESP-12F). Se conecta directamente al puerto de servicio USB-A de la unidad interior e incluye todo lo necesario para la comunicación: conector USB-A, regulación de 5 V y adaptación de niveles entre la lógica de 3,3 V del ESP8266 y la UART TTL de 5 V del aire acondicionado (UART del lado del equipo: **GPIO12 = TX, GPIO14 = RX**). No requiere cables ni piezas adicionales.
- Es probable que también funcionen **otras placas ESP8266 o diseños propios**, pero deberá implementar la misma circuitería auxiliar. Este repositorio no incluye un diseño de referencia; el SLWF-01 es la opción más sencilla.
- El puerto de servicio es una UART TTL de 5 V y 4 pines, **no** un dispositivo USB: no lo conecte a una computadora.


## Instalación

### Opción 1: instalar la imagen publicada (sin herramientas de compilación)

Con cada [versión](https://github.com/adamgranted/esphome-della-ac/releases) se publica una imagen precompilada sin credenciales.
Conecte el SLWF-01 a su equipo mediante USB e **[instálelo desde el navegador](https://adamgranted.github.io/esphome-della-ac/)**
(Chrome o Edge), o descargue `della-ac-esp8266.factory.bin` y grábelo mediante
`esptool.py write_flash 0x0 della-ac-esp8266.factory.bin`.

Al iniciarse por primera vez, el módulo aún no tiene configurada una red Wi-Fi. La herramienta del navegador ofrece el paso **Configurar Wi-Fi** inmediatamente después de la instalación, a través de la misma conexión USB-C. Como alternativa, conéctese al punto de acceso **`AC-wifi`** que crea el módulo (contraseña `slwf01pro`) y seleccione su red en el portal cautivo. Después, conecte el módulo al puerto de servicio del aire acondicionado y añádalo a Home Assistant; configure su propia clave de API y contraseña OTA durante el proceso.

Una vez conectado a su red, podrá acceder a un panel de depuración, con los estados de las entidades y los registros en directo, desde la dirección IP del dispositivo.

### Opción 2: compilar desde el código fuente

1. Instale [ESPHome](https://esphome.io/) (versión 2026.5.3 verificada).
2. Copie la plantilla de credenciales y complétela: `cp secrets.yaml.example secrets.yaml`.
3. Compile y grabe el firmware (USB la primera vez y OTA en adelante): `esphome run della-slwf.yaml`.
4. Acepte el dispositivo que la integración de **ESPHome** descubre automáticamente en Home Assistant;
   la entidad de climatización `Della AC` y los sensores de telemetría aparecerán en la página del dispositivo.

Ambas compilaciones comparten [`della-ac.base.yaml`](della-ac.base.yaml): `della-slwf.yaml`
incorpora sus credenciales y `della-ac.factory.yaml` es la imagen publicada sin credenciales.


## Cómo funciona

La recepción utiliza un `remote_receiver` de ESPHome para capturar los tiempos de los pulsos. Una función lambda los reconstruye como bytes a 4800 baudios, 8E1, y comprueba su CRC. Un pequeño componente local (`components/della_ac/`) convierte esos bytes en una entidad de climatización de Home Assistant y construye los comandos salientes. La lógica del protocolo se mantiene deliberadamente en la función lambda de YAML, mientras que C++ contiene solo la capa de integración con el termostato de Home Assistant. Consulte
[`docs/PROTOCOL.md`](docs/PROTOCOL.md) para conocer el formato de las tramas, los mapas de campos, el CRC y las particularidades que compensa el firmware.


## Estructura del repositorio

| Ruta | Qué es |
|------|------|
| `della-ac.base.yaml` | Cuerpo común del firmware (decodificación RX, sondeo y entidades de climatización y telemetría) |
| `della-slwf.yaml` | Compilación personal: base + sus credenciales (ejecute `esphome run` con este archivo) |
| `della-ac.factory.yaml` | Imagen publicada sin credenciales: base + punto de acceso `AC-wifi` para el emparejamiento |
| `components/della_ac/` | Componente local de climatización para ESPHome (capa de integración con el termostato de Home Assistant) |
| `della-la.yaml` | Compilación básica de "analizador lógico": volcado de pulsos sin procesar para analizar el protocolo |
| `analyze_bursts.py`, `pulse2bytes.py` | Decodificadores sin conexión: pulsos del registro → bytes → CRC |
| `della_ctl.py` | Control mediante la API nativa (listar / activar / configurar entidades) |
| `test_ladder.py` | Secuencia de verificación de control mediante script sobre la API nativa |
| `ir_sweep.py` | Captura guiada: pulse un botón del mando a distancia y observe qué campo decodificado cambia |
| `docs/PROTOCOL.md` | El protocolo AUX: descubrimiento, tramas, mapas de campos, particularidades |


## Créditos

- [GrKoR/AUX_HVAC_Protocol](https://github.com/GrKoR/AUX_HVAC_Protocol) y
  [GrKoR/esphome_aux_ac_component](https://github.com/GrKoR/esphome_aux_ac_component) —
  la documentación del protocolo AUX y la implementación de referencia en las que se basa este trabajo.
- [dudanov/iot-uni-dongle](https://github.com/dudanov/iot-uni-dongle) — referencia de
  hardware del dongle.


## Licencia

[GPL-3.0-only](LICENSE) — Copyright (C) 2026 adamgranted. El firmware enlaza con el entorno de ejecución C++ de ESPHome, publicado bajo GPLv3, y se basa en la [implementación de referencia](https://github.com/GrKoR/esphome_aux_ac_component) de GrKoR, también bajo GPLv3. Por tanto, el firmware distribuido se ofrece bajo la licencia GPLv3.
