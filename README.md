# Deauther 2.4GHz 5GHz com BW16 - 2.0

Dispositivo desenvolvido para **auditoria e testes de segurança** em redes Wi-Fi de 5GHz e 2.4GHz.

> ## ⚠️ AVISO IMPORTANTE — LEIA ANTES DE USAR
>
> **Este projeto existe exclusivamente para fins educacionais, de estudo de vulnerabilidades e de cibersegurança.**
>
> - Use **somente em ambientes controlados** e em **redes que são suas** ou para as quais você tem **autorização explícita e por escrito**.
> - Atacar, interferir ou derrubar redes de terceiros **sem autorização é ilegal** na maioria dos países e pode resultar em responsabilização civil e criminal.
> - O autor **não se responsabiliza** por qualquer uso indevido deste material.
>
> Se você não tem certeza se pode usar, **não use**.

## Descrição

O **Deauther BW16** é um dispositivo portátil de auditoria de redes Wi-Fi construído em torno da placa **Bw16 (RTL8720DN)**, que conta com rádio *dual-band* — ou seja, opera tanto em **2.4GHz quanto em 5GHz**, algo que a maioria dos deauthers baseados em ESP8266/ESP32 não alcança. Alimentado por bateria e com interface própria (display OLED + botões), funciona de forma totalmente autônoma, sem precisar de computador.

Na prática, ele permite escanear os pontos de acesso ao redor, selecionar um ou vários alvos e executar testes de **desautenticação (deauth)**, que derrubam temporariamente a conexão dos dispositivos com a rede, além de ataques de **Beacon Flood** (geração de redes falsas) e do modo combinado **Beacon + Deauth**. Possui ainda um **modo Monitor**, que exibe em tempo real um gráfico do tráfego e a contagem de pacotes por canal, útil para observar o comportamento do espectro Wi-Fi.

O objetivo é servir como ferramenta de **estudo de vulnerabilidades e cibersegurança**, demonstrando na prática as fragilidades dos frames de gerenciamento do padrão 802.11 — sempre em ambientes controlados e autorizados.

## Funcionalidades

- Varredura de pontos de acesso Wi-Fi próximos.
- Seleção múltipla.
- Ataques de Desautenticação (Deauth) em redes selecionadas ou em todas as redes visíveis.
- Ataques de Beacon Flood e Beacon + Deauth.
- Modo Monitor para visualização gráfica e contagem de pacotes em tempo real.
- Interface gráfica operada por display OLED e botões de navegação.

## Hardware

O projeto utiliza a placa **Bw16 (RTL8720DN)**, alimentada por um sistema de bateria recarregável, abrigado em uma case impressa em 3D.

### Componentes Utilizados
- Placa Bw16 (RTL8720DN)
- Display OLED SSD1306 I2C (128x64)
- 3x Botões Push (Tactile Switch)
- Bateria LiPo 650mAh 3.7V
- Módulo Carregador TP4056 (USB Tipo-C)
- Interruptor deslizante micro mini SPDT
- Case customizada impressa em 3D

## Software

Para gravar o dispositivo você precisa apenas do **sketch** e das **bibliotecas corrigidas** — não é necessário baixar o repositório inteiro.

**Downloads diretos (baixam só a pasta, já em `.zip`):**

- 📥 [Baixar o sketch `oled_deauther`](https://download-directory.github.io/?url=https%3A%2F%2Fgithub.com%2Fgabriel-re-pires%2FDeauther%2Ftree%2Fmain%2Foled_deauther) — é o que você abre no Arduino IDE.
- 📥 [Baixar as bibliotecas `RTL8720DN_SSD1306_FIX`](https://download-directory.github.io/?url=https%3A%2F%2Fgithub.com%2Fgabriel-re-pires%2FDeauther%2Ftree%2Fmain%2FRTL8720DN_SSD1306_FIX) — versões do display ajustadas para a BW16.

**Como gravar na placa:**

1. Instale o [Arduino IDE](https://www.arduino.cc/en/software).
2. Adicione o core da placa: **Realtek Ameba (AmebaD / RTL8720DN)** pelo Gerenciador de Placas. Instruções oficiais em [ambiot/ambd_arduino](https://github.com/ambiot/ambd_arduino).
3. Baixe e descompacte o sketch `oled_deauther` (link acima).
4. Instale as bibliotecas do `RTL8720DN_SSD1306_FIX`: copie as pastas para a sua pasta de bibliotecas do Arduino (`Documentos/Arduino/libraries`) ou use **Sketch → Incluir Biblioteca → Adicionar biblioteca .ZIP**.
5. Abra o arquivo `oled_deauther.ino` no Arduino IDE.
6. Selecione a placa **BW16 (RTL8720DN)** e a porta COM correta.
7. Clique em **Upload** para gravar na placa.

## Medida defensiva

Este dispositivo explora uma fragilidade dos **frames de gerenciamento 802.11** (deauth/beacon), que em redes WPA/WPA2 **não são autenticados**. Se você quer proteger a sua rede contra esse tipo de ataque, tome as seguintes medidas:

- **Ative WPA3 e/ou 802.11w (PMF – Protected Management Frames):** essa é a defesa mais eficaz. Ela faz com que os frames de desautenticação passem a ser autenticados, neutralizando o ataque de deauth.
- **Use conexão cabeada (Ethernet)** para dispositivos críticos — o ataque afeta apenas o Wi-Fi.
- **Mantenha o firmware do roteador atualizado** e habilite as proteções oferecidas pelo fabricante.
- **Monitore desconexões súbitas e repetidas** ou o surgimento de redes/pontos de acesso desconhecidos (*rogue AP*): podem indicar um ataque de deauth/beacon em andamento.
- **Lembre-se:** o deauth é uma **negação de serviço temporária**. Em redes WPA2/WPA3 ele **não captura senhas nem dados** — apenas derruba a conexão enquanto o ataque está ativo.

## Imagens do Dispositivo

<p align="center">
  <img src="img/device1.png" width="40%" alt="Demonstração 1 do Dispositivo">
  <img src="img/device2.png" width="40%" alt="Demonstração 2 do Dispositivo">
</p>
