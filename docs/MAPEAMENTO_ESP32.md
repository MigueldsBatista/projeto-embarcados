# RC522 (MFRC522) + ESP32 (PlatformIO)

Este projeto usa o leitor RFID RC522 via **SPI** para ler o UID do cracha e mapear para informacoes (nome/cargo) no firmware.

## Pinos do RC522 (como no modulo da foto)

Ordem do conector do RC522 (da esquerda para a direita):

`SDA  SCK  MOSI  MISO  IRQ  GND  RST  3V3`

Notas:
- `SDA` no RC522 e o **SS/CS** (chip select) do SPI.
- `IRQ` e opcional (pode deixar desconectado).
- Alimente o RC522 com **3V3** (nao use 5V).

## Ligacao recomendada (RC522 -> ESP32)

| RC522 | Funcao | ESP32 (GPIO) |
|------|--------|--------------|
| 3V3  | Alimentacao | 3V3 |
| GND  | Terra | GND |
| SCK  | SPI SCLK | GPIO18 |
| MOSI | SPI MOSI | GPIO23 |
| MISO | SPI MISO | GPIO19 |
| SDA  | SPI CS/SS | GPIO5 |
| RST  | Reset | GPIO22 |
| IRQ  | Interrupcao | (nao ligar) |

Se seus pinos `GPIO5` ou `GPIO22` estiverem ocupados no seu hardware, voce pode trocar e atualizar no `platformio.ini` (ver abaixo).

## Configuracao no PlatformIO

O projeto define os pinos assim (em `platformio.ini`):

`-D RC522_SS_PIN=5`

`-D RC522_RST_PIN=22`

Para alterar:
1. Edite `platformio.ini`
2. Troque os valores de `RC522_SS_PIN` e `RC522_RST_PIN`
3. Recompile e envie para a placa

## Subir o firmware e monitorar

Comandos:

```bash
pio run -t upload
pio device monitor -b 9600
```

Voce deve ver:
- `RC522 pronto. Aproxime o cracha...`

Ao aproximar um cracha:
- `UID: XX:XX:XX:XX` (ou com mais bytes, depende do tipo do cartao)

## Problemas comuns

1. Nao aparece UID
Verifique:
- Alimentacao em `3V3`
- `GND` comum entre ESP32 e RC522
- Pinos `SCK/MOSI/MISO` corretos (18/23/19)
- `SDA` indo para o CS (GPIO5 por padrao)
- Baud do monitor (`9600` no projeto)

2. Leitura instavel
Verifique:
- Cabos curtos
- RC522 longe de fontes de ruido
- Boa alimentacao 3V3
