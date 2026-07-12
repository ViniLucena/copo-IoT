# Copo Inteligente - Bar Inteligente IoT

Este repositório contém o desenvolvimento do módulo **Copo Inteligente**, parte do projeto final da disciplina de Internet das Coisas.

O projeto integra hardware embarcado, firmware para ESP32, comunicação MQTT, fluxos Node-RED, banco de dados PostgreSQL/Timescale, dashboards Grafana, bot Telegram, servidor, PCB e peças impressas em 3D.

---

## Visão geral

O **Copo Inteligente** é um módulo acoplado a um copo térmico de 600 mL. Ele mede informações relacionadas ao estado do copo, envia os dados ao sistema central e reage visualmente a eventos do sistema, como gols durante partidas.

O módulo faz parte de um sistema maior de **Bar Inteligente para Eventos Esportivos**, no qual diferentes dispositivos e serviços se comunicam para registrar consumo, associar copos e clientes, exibir dashboards e gerar alertas.

---

## Funcionalidades

A versão atual do módulo possui as seguintes funcionalidades:

- medição aproximada da quantidade de bebida no copo;
- medição de temperatura;
- monitoramento do nível de bateria;
- envio periódico de dados via MQTT;
- integração com Node-RED;
- armazenamento de dados em PostgreSQL/Timescale;
- visualização de dados em Grafana;
- recepção de alertas MQTT;
- animação de LEDs RGB para eventos de gol;
- integração com RFID dentro do sistema;
- estrutura física impressa em 3D para acomodar PCB, sensores e bateria.

---

## Hardware

O protótipo utiliza:

- ESP32-S3;
- sensor de distância ToF VL53L0X;
- sensor de temperatura DS18B20;
- sensor INA219 para monitoramento da bateria;
- LEDs endereçáveis WS2812B;
- bateria 18650;
- suporte para bateria 18650;
- PCB própria;
- módulo/chip RFID;
- case impressa em 3D.

O sensor de peso com célula de carga/HX711 foi considerado durante o desenvolvimento, mas não foi incluído na versão final do módulo.

---

## Copo utilizado

O recipiente usado é um copo térmico de aço inoxidável com tampa.

Características principais:

- capacidade: 600 mL;
- altura: 17,5 cm;
- diâmetro superior: 8,7 cm;
- diâmetro inferior: 7,3 cm;
- material: aço inoxidável;
- parede dupla com isolamento térmico.

A estimativa da quantidade de bebida é feita por meio da distância medida pelo sensor ToF entre o topo do copo e a superfície detectada.

---

## Firmware

O firmware do copo é responsável por:

- conectar o ESP32 ao WiFi;
- conectar ao broker MQTT com TLS;
- ler os sensores;
- calcular a quantidade aproximada de bebida;
- medir temperatura;
- monitorar bateria;
- publicar dados do copo;
- receber mensagens de alerta;
- controlar os LEDs.

Os principais dados publicados pelo copo seguem o formato:

```json
{
  "id_copo": 1,
  "quantidade_ml": 350.0,
  "temperatura_c": 24.3
}
```

O tópico principal de publicação segue o padrão:

```text
copo/{ID_COPO}/dados
```

O nível da bateria é publicado em tópico próprio, e o copo também recebe alertas pelo tópico utilizado nos fluxos do Node-RED.

---

## LEDs e alertas

Os LEDs WS2812B são usados tanto para indicar o nível aproximado de bebida quanto para alertas visuais.

Quando uma mensagem de gol é recebida, o firmware identifica o time mencionado e anima os LEDs com as cores associadas a esse time.

Exemplo de mensagem de alerta:

```json
{
  "mensagem": "Gol do Brasil",
  "target_device": 1
}
```

Mensagens como `parar`, `stop` ou `normal` encerram o alerta visual.

---

## Backend e dashboards

A integração com o backend é feita usando:

- MQTT para comunicação;
- Node-RED para tratamento e roteamento das mensagens;
- PostgreSQL/Timescale para armazenamento;
- Grafana para visualização;
- servidor e bot Telegram para integração com o restante do sistema.

O repositório inclui os fluxos Node-RED, as queries do Grafana e demais arquivos necessários para reproduzir a integração.

Entre os dados exibidos no Grafana estão:

- quantidade atual por copo;
- temperatura;
- nível de bateria;
- tempo desde que o copo ficou vazio;
- informações integradas aos demais módulos do bar inteligente.

---

## Estrutura física

A estrutura do copo foi projetada para impressão 3D e montagem lateral no copo térmico.

A case acomoda:

- PCB;
- bateria 18650;
- passagem de fios até o topo do copo;
- sensor ToF;
- sensor de temperatura;
- RFID;
- LEDs e conexões auxiliares.

Como o copo é metálico, o RFID deve ficar posicionado na parte externa da case, afastado da parede de inox, para reduzir interferência na leitura.

---

## Organização do repositório

O repositório contém arquivos relacionados a:

- firmware do copo;
- códigos de sensores, LEDs, bateria e MQTT;
- fluxos Node-RED;
- queries do Grafana;
- arquivos de PCB e esquemáticos;
- peças STL da case;
- servidor;
- bot Telegram;
- documentação complementar.

A organização exata pode ser consultada diretamente nas pastas do repositório.

---

## Testes realizados

Durante o desenvolvimento foram realizados testes de:

- leitura do sensor ToF;
- calibração aproximada da quantidade de bebida;
- leitura do sensor DS18B20;
- leitura do nível de bateria com INA219;
- acionamento dos LEDs WS2812B;
- envio e recepção de mensagens MQTT;
- integração com Node-RED;
- gravação e atualização dos dados no banco;
- visualização em Grafana;
- montagem da PCB e da estrutura impressa em 3D.

---

## Status atual

O protótipo atual integra hardware, firmware, backend e visualização.

Principais elementos desenvolvidos:

- leitura de nível por ToF;
- leitura de temperatura;
- monitoramento de bateria;
- LEDs de alerta;
- comunicação MQTT;
- fluxos Node-RED;
- banco de dados;
- dashboards Grafana;
- PCB;
- peças 3D;
- integração com servidor e bot Telegram.

---

## Autores

Projeto desenvolvido como parte da disciplina de Internet das Coisas.

Módulo: **Copo Inteligente**  
Sistema geral: **Bar Inteligente IoT para Eventos Esportivos**
