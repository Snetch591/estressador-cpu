Estressador de CPU criado em C

Como compilar e executar (Code: :Blocks é necessário pois não consegui descobrir como executar sem o Code: :Blocks):
1. Baixe ou clone este repositório.
2. Abra o Code::Blocks.
3. Crie um novo projeto Win32 GUI Project.
4. Substitua o conteúdo do arquivo principal pelo código no repositório.
5. Vá em Project → Build Options → Linker settings e adicione `-lm` em "Other linker options".
6. Compile (F9) e execute.

3,1. Ou crie um novo file c source.

3,2.Copie o código fornecido no GitHub no novo file.

3,3. Clique em build and run.

Como usar:
1. Informe o Número de Threads (recomendado: igual ao número de núcleos).
2. Defina a Intensidade (1 = leve, 10 = muito pesado).
3. Coloque a Duração em segundos (0 = infinito).
4. Clique em INICIAR ESTRESSE.
5. Monitore o status em tempo real.
6. Clique em PARAR ESTRESSE ou feche a janela para encerrar.

O que são estressadores de CPU e como funcionam:
Estressadores de CPU (também chamados de *CPU Burners* ou *Stress Testers*) são programas projetados para maximizar o uso da CPU de forma controlada.
Princípios de funcionamento:
Multi-threading: Criam uma thread por núcleo (ou mais) para utilizar todos os núcleos simultaneamente.
- Cálculos Intensivos: Utilizam operações matemáticas pesadas (`sin()`, `cos()`, `sqrt()`, `tan()`, etc.) dentro de loops apertados para forçar o processador a trabalhar no máximo.
- `volatile`: Impede que o compilador otimize o código e remova os cálculos.
- Monitoramento: Verificam periodicamente condições de parada (tempo, tecla pressionada ou flag).
- Objetivos comuns:
  - Testar estabilidade térmica e de tensão
  - Verificar overclocks
  - Testar sistema de refrigeração
  - Identificar problemas de hardware
