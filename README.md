# Copo Inteligente - Bar Inteligente IoT

Este repositório contém a documentação e o desenvolvimento do módulo Copo Inteligente, parte do projeto final da disciplina de Internet das Coisas.

O projeto geral consiste em um Bar Inteligente para Eventos Esportivos, integrando copos inteligentes, torneira automatizada, sistema de gamificação por mesas, MQTT, Node-RED, Timescale/PostgreSQL, Grafana, PCB e impressão 3D.

## 1. Visão Geral

O Copo Inteligente é um módulo físico acoplado ao copo por meio de uma estrutura externa, chamada no projeto de camisinha da cerveja. Essa estrutura abriga os sensores, LEDs, circuito eletrônico e sistema de identificação do copo.

O objetivo do módulo é monitorar informações relacionadas ao consumo e ao estado do copo, como:

- identificação do copo;
- associação do copo a uma mesa ou cliente;
- quantidade aproximada de bebida;
- temperatura da bebida;
- presença correta do copo na base/suporte;
- alertas visuais por LEDs;
- envio dos dados para o sistema central via MQTT.

Essas informações são utilizadas pelos outros módulos do sistema, como a torneira inteligente, o totem da mesa, o sistema de pontuação/apostas e o dashboard em Grafana.

## 2. Objetivos do Módulo

O módulo Copo Inteligente tem como principais objetivos:

- registrar o copo no sistema por meio de RFID;
- medir a quantidade de bebida presente no copo;
- medir a temperatura da bebida ou da superfície do copo;
- detectar se o copo está corretamente posicionado na base;
- indicar estados do sistema por meio de LEDs;
- enviar leituras periódicas para o backend;
- integrar-se com o restante do Bar Inteligente via MQTT;
- armazenar as leituras em banco de dados Timescale/PostgreSQL;
- permitir visualização dos dados em Grafana.

## 3. Funcionalidades Previstas

### 3.1 Identificação por RFID

Cada copo possui uma tag RFID fixa, que permite sua identificação pelo sistema.

A tag pode ser lida por outros módulos, como:

- totem da mesa;
- torneira inteligente;
- estação de cadastro;
- sistema de autenticação.

A partir do RFID do copo, o backend pode associar o copo a uma mesa, cliente ou sessão ativa.

### 3.2 Medição de Quantidade

O copo pode utilizar um ou mais sensores para estimar a quantidade de bebida presente.

Opções consideradas:

- sensor de nível líquido tipo e-tape;
- sensor de peso/célula de carga;
- combinação de sensores para maior confiabilidade.

A quantidade estimada pode ser enviada em mililitros ou em porcentagem de capacidade.

### 3.3 Medição de Temperatura

Um sensor de temperatura é utilizado para monitorar a temperatura da bebida ou da parede externa do copo.

Essa informação pode ser usada para:

- indicar se a bebida está na temperatura ideal;
- gerar alertas no dashboard;
- ativar LEDs no copo;
- criar regras de gamificação ou promoções.

### 3.4 Sensor de Presença/Proximidade

Um sensor de proximidade detecta se o copo está corretamente colocado na base ou na camisinha da cerveja.

Essa informação evita leituras inválidas quando o copo não está bem posicionado.

Exemplo de estados:

- copo presente;
- copo ausente;
- copo mal posicionado.

### 3.5 LEDs de Estado

O módulo possui LEDs ou fita LED para indicar visualmente o estado do copo.

Sugestão de lógica:

| Cor / Estado | Significado                                            |
| ------------ | ------------------------------------------------------ |
| Azul         | Copo cadastrado                                        |
| Verde        | Bebida em temperatura adequada                         |
| Amarelo      | Baixa quantidade de bebida                             |
| Vermelho     | Erro, copo não cadastrado ou mal posicionado           |
| Piscando     | Evento especial, gol, aposta vencida ou promoção ativa |

## 4. Arquitetura do Sistema

A comunicação principal do Copo Inteligente com o sistema é feita via MQTT.

Fluxo simplificado:

Sensores do copo
↓
ESP32
↓ MQTT
Node-RED
↓
Timescale/PostgreSQL
↓
Grafana

O sistema também pode receber comandos externos:

Totem / Telegram / Node-RED
↓ MQTT
ESP32 do Copo
↓
LEDs / Alertas visuais

## 5. Hardware Utilizado

Componentes previstos para o módulo:

- ESP32;
- RFID tag;
- sensor de quantidade:
  - e-tape liquid level sensor, ou
  - célula de carga com HX711;
- sensor de temperatura;
- sensor de proximidade;
- fita LED ou LED RGB;
- PCB projetada no EasyEDA;
- estrutura impressa em 3D para fixação no copo;
- fonte de alimentação ou bateria para protótipo.

## 6. Estrutura Física

A eletrônica do módulo será integrada em uma estrutura externa acoplada ao copo, chamada de camisinha da cerveja.

Essa estrutura tem as seguintes funções:

- proteger a eletrônica;
- posicionar corretamente os sensores;
- permitir contato adequado com o copo;
- comportar LEDs visíveis externamente;
- facilitar o uso em uma mesa de bar;
- servir como suporte físico para a PCB.

A estrutura será modelada em CAD e fabricada por impressão 3D.

## 7. Tópicos MQTT

Sugestão de tópicos MQTT utilizados pelo módulo:

copo/{id_copo}/status
copo/{id_copo}/quantidade
copo/{id_copo}/temperatura
copo/{id_copo}/evento
copo/{id_copo}/comando
mesa/{id_mesa}/alerta

Exemplo de publicação de status:

```json
{
  "id_copo": 12,
  "rfid_copo": "A0 B1 C2 D3",
  "mesa": 3,
  "cliente": "Cliente 1",
  "quantidade_ml": 280,
  "temperatura": 4.8,
  "posicionado": true
}
```

Exemplo de comando recebido:

```json
{
  "evento": "gol",
  "cor": "verde",
  "piscar": true
}
```

## 8. Banco de Dados

O módulo Copo Inteligente se relaciona principalmente com as tabelas de clientes, copos, cadastro e leituras.

### 8.1 Tabela de Clientes

```SQL
CREATE TABLE clientes (
id SERIAL PRIMARY KEY,
nome TEXT NOT NULL,
rfid_cliente TEXT UNIQUE
);
```

### 8.2 Tabela de Copos

```SQL
CREATE TABLE copos (
id SERIAL PRIMARY KEY,
rfid_copo TEXT UNIQUE NOT NULL,
mesa_id INTEGER,
ativo BOOLEAN DEFAULT TRUE
);
```

### 8.3 Tabela de Cadastro Copo-Cliente

```SQL
CREATE TABLE cadastro_copo_cliente (
id SERIAL PRIMARY KEY,
cliente_id INTEGER,
copo_id INTEGER,
mesa_id INTEGER,
inicio TIMESTAMPTZ DEFAULT NOW(),
fim TIMESTAMPTZ
);
```

### 8.4 Tabela de Leituras do Copo

```SQL
CREATE TABLE leituras_copo (
time TIMESTAMPTZ DEFAULT NOW(),
copo_id INTEGER,
mesa_id INTEGER,
quantidade_ml REAL,
temperatura REAL,
posicionado BOOLEAN
);
```

## 9. Integração com os Outros Módulos

### 9.1 Integração com a Torneira Inteligente

A torneira pode ler o RFID do copo antes de liberar a bebida.

Fluxo esperado:

RFID do copo lido na torneira
↓
Verificação no backend
↓
Autorização de abastecimento
↓
Medição do volume servido
↓
Atualização da conta e do consumo

### 9.2 Integração com o Totem da Mesa

O totem pode ser usado para cadastrar copos em uma mesa.

Fluxo esperado:

Copo aproximado do leitor RFID do totem
↓
Totem identifica o copo
↓
Sistema associa copo à mesa
↓
Copo passa a receber alertas daquela mesa

### 9.3 Integração com o Sistema de Apostas/Pontuação

O copo pode receber alertas visuais relacionados ao andamento do jogo ou à pontuação da mesa.

Exemplos:

- piscar LEDs quando a mesa ganha pontos;
- mudar cor em caso de gol;
- indicar promoção ativa;
- indicar prêmio ou desconto disponível.

## 10. Grafana

As leituras do copo podem ser visualizadas em dashboards no Grafana.

Possíveis gráficos:

- quantidade média de bebida por mesa;
- temperatura da bebida ao longo do tempo;
- número de copos ativos;
- copos mal posicionados;
- consumo por mesa;
- histórico de eventos por copo.

Exemplo de consulta para temperatura média:

```SQL
SELECT
time_bucket('1 minute', time) AS time,
AVG(temperatura) AS temperatura_media
FROM leituras_copo
WHERE $\_\_timeFilter(time)
GROUP BY time
ORDER BY time ASC;
```

Exemplo de consulta para quantidade média por mesa:

```SQL
SELECT
time_bucket('1 minute', time) AS time,
mesa_id,
AVG(quantidade_ml) AS quantidade_media
FROM leituras_copo
WHERE $\_\_timeFilter(time)
GROUP BY time, mesa_id
ORDER BY time ASC;
```

## 11. Node-RED

O Node-RED será responsável por integrar o MQTT com o banco de dados e com os outros módulos.

Fluxo básico esperado:

MQTT In
↓
JSON
↓
Change
↓
PostgreSQL
↓
Debug

Funções principais:

- receber dados publicados pelo ESP32;
- converter mensagens JSON;
- inserir leituras no banco de dados;
- enviar comandos para os copos;
- encaminhar alertas do sistema de apostas;
- integrar com Telegram, se utilizado.

## 12. Organização do Repositório

Sugestão de estrutura:

```text
copo-inteligente/
├── firmware/
│ └── copo_inteligente/
│ └── copo_inteligente.ino
├── pcb/
│ ├── esquematico.pdf
│ ├── pcb_layout.pdf
│ └── arquivos_easyeda/
├── 3d/
│ ├── modelo_camisinha_cerveja.stl
│ └── imagens/
├── node-red/
│ └── flows.json
├── database/
│ └── create_tables.sql
├── docs/
│ ├── arquitetura.md
│ ├── mqtt.md
│ └── testes.md
└── README.md
```

## 13. Etapas de Desenvolvimento

Semana 1

- definição dos sensores utilizados;
- definição da arquitetura do módulo;
- início da organização no Trello;
- testes iniciais com ESP32;
- definição dos tópicos MQTT.

Semana 2

- desenvolvimento do circuito;
- início do projeto da PCB no EasyEDA;
- testes de leitura dos sensores;
- modelagem inicial da estrutura 3D.

Semana 3

- integração com MQTT e Node-RED;
- criação das tabelas no PostgreSQL/Timescale;
- envio de dados reais para o banco;
- primeiros gráficos no Grafana.

Semana 4

- finalização do protótipo;
- documentação no GitHub;
- testes integrados com os outros módulos;
- preparação da demonstração final.

## 14. Testes Planejados

Testes previstos:

- leitura correta do RFID do copo;
- medição de quantidade com copo vazio, meio cheio e cheio;
- medição de temperatura;
- detecção de copo presente/ausente;
- envio correto de JSON por MQTT;
- gravação das leituras no banco de dados;
- visualização das leituras no Grafana;
- acionamento correto dos LEDs;
- integração com totem e torneira.

## 15. Status Atual

- Definição final dos sensores
- Teste do RFID
- Teste do sensor de quantidade
- Teste do sensor de temperatura
- Teste do sensor de proximidade
- Teste dos LEDs
- Comunicação MQTT
- Fluxo Node-RED
- Tabelas PostgreSQL/Timescale
- Dashboard Grafana
- PCB no EasyEDA
- Modelo 3D da camisinha da cerveja
- Integração com os outros grupos
- Documentação final

## 16. Autores

Projeto desenvolvido como parte da disciplina de Internet das Coisas.

Módulo: Copo Inteligente
Sistema geral: Bar Inteligente IoT para Eventos Esportivos

```

```
