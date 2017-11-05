# ProtocoloDeEnlace

Um protocolo de comunicação está relacionado aos mecanismos necessários para a entrega de mensagens entre duas aplicações quaisquer. Considerando uma arquitetura de redes em camadas como TCP/IP, protocolos de comunicação correspondem às camadas de enlace até transporte. Questões como garantia de entrega, controle de sequência, tratamento de erros, sincronização, estabelecimento e término de sessão, multiplexação e delimitação de mensagens, entre possivelmente outras, fazem parte do projeto de tais protocolos. Para introduzir o projeto de um protocolo de comunicação, o primeiro projeto da disciplina envolve um protocolo para estabelecimento de enlace sem-fio ponto-a-ponto.
Considere o caso de uma nova interface de rede sem-fio composta por um transceiver RF capaz de transmitir a distâncias de até 1 km. No caso de distâncias como essa, a taxa de transmissão possível de ser obtida é de 2400 bps, porém distâncias menores possibilitam taxas maiores, até um máximo de 19200 bps. Esse transceiver pode ser usado como uma interface serial do tipo UART. Portanto, com ele podem-se criar enlaces de média distância e baixas taxas de transmissão.
O transceiver RF usado como UART proporciona uma camada física, cuja interface de acesso a serviço oferece operações de envio e recepção de bytes. Nenhuma facilidade para delimitação de mensagens, endereçamento, sincronização e tratamento de erros é fornecida. De fato, tais serviços devem ser implementados em um protocolo de enlace que use esse transceiver como camada física.
O projeto 1 envolve o desenvolvimento de um protocolo de comunicação usando esse transceiver RF, de forma a oferecer um serviço de comunicação com essas características. O projeto é desenvolvido por uma equipe de futuros Engenheiros de Telecomunicações, sendo eles: André Felippe Weber, Gabriel Cozer Cantu e Maria Luiza Theisges.
