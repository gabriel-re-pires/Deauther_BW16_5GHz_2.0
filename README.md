# Deauther 2.4GHz 5GHz com BW16 - 2.0

Dispositivo desenvolvido para **auditoria e testes de segurança** em redes Wi-Fi de 5GHz e 2.4GHz.

> ## AVISO
>
> **Este projeto existe exclusivamente para fins educacionais, de estudo de vulnerabilidades e de cibersegurança.**
>
> - Use **somente em ambientes controlados** e em **redes que são suas** ou as quais você tem **autorização**.
> - Atacar, interferir ou derrubar redes de terceiros **sem autorização é ilegal**.
> - O autor **não se responsabiliza** por qualquer uso indevido deste material.
>
> Se você não tem certeza se pode usar, **não use**.

## Descrição

**Deauther** é um dispositivo portátil de auditoria de redes Wi-Fi feito em torno da placa **Bw16(RTL8720DN)**, *dual-band*, operando em **2.4GHz e 5GHz**.

Ele escanear os pontos de acesso ao redor, selecionar um ou vários e executar testes de **desautenticação (deauth)**, que derrubam temporariamente a conexão dos dispositivos com a rede, além de **Beacon Flood** (geração de redes falsas). Possui ainda um **modo Monitor**, que exibe em tempo real um gráfico do tráfego e a contagem de pacotes por canal, útil para observar o comportamento do espectro Wi-Fi.

Serve como ferramenta de **estudo de vulnerabilidades e cibersegurança**, demonstrando na prática as fragilidades dos frames de gerenciamento do padrão 802.11. Sempre em ambientes controlados e autorizados.

**O Deauth é uma negação de serviço temporária**. Em redes WPA2/WPA3 ele **não captura senhas nem dados**, apenas derruba a conexão enquanto o ataque está ativo.

## Funcionalidades

- Varredura de pontos de acesso Wi-Fi próximos.
- Seleção múltipla.
- Ataques de Desautenticação (Deauth) em redes selecionadas ou em todas as redes visíveis.
- Ataques de Beacon Flood e Beacon + Deauth.
- Modo Monitor para visualização gráfica e contagem de pacotes em tempo real.
- Interface gráfica operada por display OLED e botões de navegação.

## Hardware

### Componentes Utilizados
- Placa Bw16 (RTL8720DN)
- Display OLED SSD1306 I2C (128x64)
- 3x Botões Push (Tactile Switch)
- Bateria LiPo 650mAh 3.7V
- Módulo Carregador TP4056 (USB Tipo-C)
- Interruptor deslizante micro mini SPDT
- Case customizada impressa em 3D

## Software

**Download (projeto completo):**

- [**Baixar o projeto completo (`.zip`)**](https://github.com/gabriel-re-pires/Deauther_BW16_5GHz_2.0/archive/refs/heads/main.zip)

**Download mínimo (o essencial para gravar — sketch + bibliotecas):**

- [**Baixar só o essencial (`.zip`)**](https://github.com/gabriel-re-pires/Deauther_BW16_5GHz_2.0/raw/main/Deauther_minimo.zip)

## Medida defensiva

Este dispositivo explora uma fragilidade dos **frames de gerenciamento 802.11** (deauth/beacon), que em redes WPA/WPA2 **não são autenticados**. Para proteger a sua rede contra esse tipo de ataque, tome as medidas:

- **Ative WPA3 e/ou 802.11w (PMF – Protected Management Frames)**.
- **Use conexão cabeada (Ethernet)**.
- **Mantenha o firmware do roteador atualizado** e habilite as proteções oferecidas pelo fabricante.
- **Monitore desconexões súbitas e repetidas** ou o surgimento de redes/pontos de acesso desconhecidos, podem indicar um ataque de deauth/beacon em andamento.

## Imagens do Dispositivo

<p align="center">
  <img src="img/device1.png" width="40%" alt="Demonstração 1 do Dispositivo">
  <img src="img/device2.png" width="40%" alt="Demonstração 2 do Dispositivo">
</p>
