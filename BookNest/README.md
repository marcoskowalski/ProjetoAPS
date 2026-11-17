# BookNest

Biblioteca pessoal e diário de leitura — C++20, Qt 6 (Widgets, Sql, Network), SQLite, CMake.

## Como abrir no CLion
1. `File > Open` e escolha esta pasta (onde está o `CMakeLists.txt`).
2. `Settings > Build, Execution, Deployment > CMake > CMake options`:
   `-DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/mingw_64` (ajuste para a sua instalação do Qt 6 com MinGW 64-bit).
3. Execute o alvo **BookNest**. Para depurar, use o botão de debug e coloque pontos de parada (ex.: `LeituraRepository::registrarProgresso`).
4. Alvo **test_core** roda os testes das regras de negócio (`ctest`).

O banco `booknest.db` é criado em `%APPDATA%/BookNest/` na primeira execução, com as 7 tabelas e 30 livros de catálogo.

## Estrutura
- `src/core` regras de negócio em C++ puro (RN001–RN011) · `src/domain` classes · `src/db` SQLite e repositórios
- `src/net` Google Books · `src/ui` janelas Qt · `resources` schema.sql e seed.sql · `tests`
- `docs` plano do projeto (docx/pdf) e diagramas · `tools` geradores (seed e diagramas)

## Enviar ao GitHub
```
git clone https://github.com/marcoskowalski/ProjetoAPS.git
# copie o conteúdo desta pasta para dentro do clone
git add . && git commit -m "BookNest: implementação completa e plano do projeto" && git push
```
(no CLion: Git > Commit… e Git > Push…)

## Funcionalidades ↔ requisitos
RF001–003 cadastro/edição/remoção · RF004 progresso · RF005 nota/resenha · RF006 filtro por status/gênero · RF007 listas · RF008 histórico/estatísticas · RF009 busca · RF010 Google Books.
