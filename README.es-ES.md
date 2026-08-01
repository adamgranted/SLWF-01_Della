

<!-- esphome-della-ac — control local de Home Assistant para aires acondicionados mini split Della -->

<div align="center">
  <a href="https://github.com/adamgranted/esphome-della-ac">
    <picture>
      <source srcset="./.github/img/della-logo-dark.svg" media="(prefers-color-scheme: dark)">
      <img src="./.github/img/della-logo-light.svg" alt="Della" height="44"/>
    </picture>
  </a>
  <h2>esphome-della-ac</h2>
  <p align="center">
      <p><b>Control local de Home Assistant para mini splits AUX-OEM Della</b></p>
  </p>

  <p align="center">
    <img alt="platform" src="https://img.shields.io/badge/platform-ESP8266-blue">
    <img alt="esphome" src="https://img.shields.io/badge/ESPHome-2026.5%2B-1c1c1c">
    <img alt="protocol" src="https://img.shields.io/badge/protocol-AUX%204800%208E1-orange">
  </p>

</div>


<br>

Firmware ESPHome que expone un mini split **AUX-OEM Della** como un termostato completo de Home Assistant: sin nube, sin cuenta de Tuya. Reemplaza el dongle Wi-Fi de fábrica por un [SMLIGHT SLWF-01](https://smlight.tech/) ejecutando este firmware, listo para usar (plug-and-play) con el puerto de servicio USB-A de Della. Desarrollado en un **Della 048-MS**; consulte [Unidades compatibles](#supported-units) para la lista de modelos.

Estas unidades **no** utilizan el protocolo TCL que sugiere su puerto USB: son hardware **AUX OEM** que habla el protocolo serial AUX HVAC a **4800 baudios, 8E1**, en una línea de drenaje abierto (open-drain). (Muestrear esa línea a los obvios 9600 baudios produce un flujo de bytes convincente pero falso; esa trampa de aliasing es la razón por la que los intentos anteriores nunca funcionaron.) La explicación completa y el mapa a nivel de bytes se encuentran en [`docs/PROTOCOL.md`](docs/PROTOCOL.md).


## Unidades compatibles

Un único firmware sirve para todos los modelos: detecta automáticamente la variante del marco de estado del aire acondicionado en tiempo de ejecución, por lo que existe **una única compilación para grabar sin importar el modelo** (no hay que elegir entre imágenes por modelo).

| Modelo | Control | Telemetría | Notas |
|-------|:------:|:--------:|-------|
| **Della 048-MS** | ✅ | ✅ | Unidad de referencia: completamente verificada |
| **Della Motto JA 12K** (`12K1VRH-20S-JA`) | ✅ | ✅ | Mismo protocolo AUX que el 048-MS (marco de estado de 35 bytes). Confirmado en capturas en reposo y enfriamiento activo ([#11](https://github.com/adamgranted/esphome-della-ac/issues/11)) |
| **Della Serena Series 18K** (`18K2VR-22S-M-I+O`) | ✅ | ✅ | Confirmado como funcional por un usuario de la comunidad ([Informe de Home Assistant](https://community.home-assistant.io/t/della-ac-integration/756819/16)) |

Es muy probable que otras unidades Della fabricadas por AUX / AUX-OEM funcionen. Si la suya no está en la lista, abra un issue con una captura de log con `verbose` activado y generalmente podrá agregarse en una o dos líneas.


## Características

- **Entidad climática completa** — off / cool / heat / dry / fan-only / heat_cool, ventilador auto / bajo / medio / alto / silencioso, oscilación vertical / horizontal / ambas, modos boost (turbo) y sleep (sueño), temperatura actual y acción HVAC
- **Controles de características** — entidades de Home Assistant para la pantalla del panel, salud (ionizador), eco, autolimpieza y ciclo de deshumidificación antifúngico, cada uno mapeado al comportamiento real del control remoto; las funciones exclusivas del estado apagado (autolimpieza, antifúngico) se gestionan como tales en lugar de ser conmutadores sin efecto
- **Sensores de telemetría y estado** — temperatura de la bobina evaporadora, compresor y exterior, % de potencia del inversor, velocidad en tiempo real del ventilador, un sensor de punto de ajuste con tipo definido y un resumen de estado legible por humanos en una sola línea
- **Recepción robusta (RX)** — recibe a través de una ruta de captura de pulsos inmune a los flancos de subida irregulares del MCU del aire acondicionado, luego reconstruye y verifica el CRC de cada trama en el dispositivo
- **Escrituras seguras** — cada comando se construye copiando la última trama de estado de la unidad y modificando solo los campos solicitados; se rechaza si la última lectura es obsoleta
- **Estado a 1 Hz** — actualizaciones en menos de 2 segundos en Home Assistant; la carga del bus se mantiene ligera
- **Únicamente local** — API nativa de ESPHome + OTA; nada sale de su red


## Hardware

- Un mini split **AUX-OEM Della** — consulte [Unidades compatibles](#supported-units) para modelos confirmados (es muy probable que otras unidades Della fabricadas por AUX funcionen).
- Un **SMLIGHT SLWF-01** (ESP-12F). Se conecta directamente al puerto de servicio USB-A de la unidad interior y ya incluye todo lo necesario para el enlace: el conector USB-A, regulación de 5 V y conversión de niveles entre la lógica de 3.3 V del ESP8266 y el UART TTL de 5 V del aire acondicionado (UART del lado del AC en **GPIO12 = TX, GPIO14 = RX**). Sin cableado, sin piezas adicionales.
- **Tarjetas ESP8266 DIY u otras** son probablemente compatibles, pero tendría que proporcionar esa misma circuitería de soporte usted mismo: no hay un diseño de referencia aquí. El SLWF-01 es la opción más sencilla.
- El puerto de servicio es un UART TTL de 5 V y 4 pines, **no** un dispositivo USB: no lo conecte a una computadora.


## Instalación

### Opción 1: grabar la compilación de lanzamiento (sin cadena de herramientas)

Con cada [lanzamiento](https://github.com/adamgranted/esphome-della-ac/releases) se publica una imagen precompilada y libre de secretos.
Conecte el SLWF-01 a su computadora por USB y **[instálelo desde su navegador](https://adamgranted.github.io/esphome-della-ac/)**
(Chrome/Edge), o descargue `della-ac-esp8266.factory.bin` y grábelo con
`esptool.py write_flash 0x0 della-ac-esp8266.factory.bin`.

Al arrancar por primera vez, el dongle no tiene Wi-Fi. Configúrelo de cualquier manera: la herramienta de grabación del navegador ofrece un paso de **Configurar Wi-Fi** justo después de la instalación (a través del mismo enlace USB-C), o únase al punto de acceso **`AC-wifi`** que crea (contraseña `slwf01pro`) y seleccione su red en el portal cautivo. Luego, conecte el dongle al puerto de servicio del aire acondicionado y adóptelo en Home Assistant: establezca su propia clave API y contraseña OTA cuando lo haga.

Una vez que esté en su red, un panel de depuración (estados de entidades + registros en vivo) se sirve en la IP del dispositivo.

### Opción 2: compilar desde el código fuente

1. Instale [ESPHome](https://esphome.io/) (2026.5.3 conocida como estable).
2. Copie la plantilla de secretos y complétela: `cp secrets.yaml.example secrets.yaml`.
3. Grabe (USB la primera vez, OTA en adelante): `esphome run della-slwf.yaml`.
4. Acepte el dispositivo que la integración de **ESPHome** descubre automáticamente en Home Assistant;
   la entidad climática `Della AC` y los sensores de telemetría aparecerán en su página de dispositivo.

Ambas compilaciones comparten [`della-ac.base.yaml`](della-ac.base.yaml) — `della-slwf.yaml`
agrega sus secretos, `della-ac.factory.yaml` es la imagen de lanzamiento libre de secretos.


## Cómo funciona

La recepción se maneja mediante un `remote_receiver` de ESPHome que captura los tiempos crudos de los pines, los cuales una lambda reconstruye en bytes de 4800 baudios 8E1 y valida mediante CRC. Un componente local pequeño (`components/della_ac/`) mapea esos bytes en una entidad climática de Home Assistant y construye los comandos salientes. Mantener la lógica del protocolo en la lambda de YAML y solo la cáscara del termostato HA en C++ es intencional; consulte
[`docs/PROTOCOL.md`](docs/PROTOCOL.md) para obtener información sobre el enmarcado, mapas de campos, CRC y las particularidades que el firmware compensa.


## Estructura del repositorio

| Ruta | Qué es |
|------|------|
| `della-ac.base.yaml` | Cuerpo compartido del firmware (decodificación RX, sondeo, entidades climáticas y de telemetría) |
| `della-slwf.yaml` | Compilación personal: base + sus secretos (`esphome run` para esto) |
| `della-ac.factory.yaml` | Imagen de lanzamiento sin secretos: base + AP `AC-wifi` para emparejamiento |
| `components/della_ac/` | Componente climático local de ESPHome (la cáscara del termostato HA) |
| `della-la.yaml` | Compilación básica de "analizador lógico": volcado crudo de pulsos para trabajo de protocolo |
| `analyze_bursts.py`, `pulse2bytes.py` | Decodificadores fuera de línea: pulsos de log → bytes → CRC |
| `della_ctl.py` | Control remoto por API nativa (listar / presionar / configurar entidades) |
| `test_ladder.py` | Secuencia de verificación de control mediante script sobre la API nativa |
| `ir_sweep.py` | Captura guiada: presione un botón del control remoto y observe el cambio en el campo decodificado |
| `docs/PROTOCOL.md` | El protocolo AUX: descubrimiento, tramas, mapas de campos, particularidades |


## Créditos

- [GrKoR/AUX_HVAC_Protocol](https://github.com/GrKoR/AUX_HVAC_Protocol) y
  [GrKoR/esphome_aux_ac_component](https://github.com/GrKoR/esphome_aux_ac_component) —
  la documentación del protocolo AUX e implementación de referencia sobre la que se basa este trabajo.
- [dudanov/iot-uni-dongle](https://github.com/dudanov/iot-uni-dongle) — referencia de
  hardware del dongle.


## Licencia

[GPL-3.0-only](LICENSE) — Copyright (C) 2026 adamgranted. El firmware vincula el entorno de ejecución C++ de ESPHome bajo GPLv3 y se basa en la [implementación de referencia](https://github.com/GrKoR/esphome_aux_ac_component) GPLv3 de GrKoR, por lo que el firmware distribuido se licencia bajo GPLv3.
