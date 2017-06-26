Spool de Impressão 

Giovani Nícolas Bettoni

giovanib@ufcspa.edu.br

Esta é a segunda tarefa da disciplina Sistemas Operacionais, ofertada pelo professor Cristiano Bonato Both na Universidade Federal de Ciências da Saúde de Porto Alegre (UFCSPA), durante o primeiro semestre de 2017. Aparentemente, tudo foi testado e está funcionando. Vou subir um print de como funciona no meu computador.

Os dois terminais do lado esquerdo estão executando `client` e estão enviando tarefas para a fila. Na parte superior, podemos ver um trabalho esperando no semáforo quando a fila está cheia. Os dois terminais do lado direito estão executando `print_server` e estão levando tarefas da fila.

`print_server` solicitará o tamanho da fila, então inicializará o espaço compartilhado e os semáforos. Depois começará a receber solicitações de impressão. Para cada trabalho, ele imprimirá as informações e aguardará o tempo correspondente. Qualquer tentativa subsequente de chamada ao 'print_server' ignorará a inicialização e começará a aceitar a solicitação de impressão imediatamente.

`client` pedirá um nome de trabalho, uma duração de trabalho (em segundos) e registrará seu PID com a estrutura do trabalho.
Conforme descrito no arquivo de cabeçalho common.h, a fila mantém as estruturas de trabalho.

A descrião do projeto (fornecida pelo professor Cristiano) está incluída no arquivo "segundo.pdf".
