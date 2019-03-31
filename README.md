# accelerometerJM

Projeto 1 : 29/03/2019

Controle de nível com visualização remota

Integrantes: Marcos Vieira e Julio Ripoll

- NodeMCU
- Sensor de aceleração (MPU-6050)

Funcionalidades:

O sensor monitora a aceleração (x, y, z) em tempo real e envia para a nuvem.
A interface mostra os componentes da aceleração em tempo real em um gráfico.
O sensor também calcula o ângulo de inclinação da componente x e envia para um segundo
NodeMCU que controla um servo-motor.