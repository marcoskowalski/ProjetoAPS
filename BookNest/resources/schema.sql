-- Esquema do BookNest (SQLite). 7 tabelas + 1 view.
-- Observação: sem ponto e vírgula dentro de textos (o script é dividido por ';').

CREATE TABLE IF NOT EXISTS autor (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    nome            TEXT    NOT NULL UNIQUE,
    nacionalidade   TEXT,
    ano_nascimento  INTEGER
);

CREATE TABLE IF NOT EXISTS genero (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    nome       TEXT NOT NULL UNIQUE,
    descricao  TEXT
);

CREATE TABLE IF NOT EXISTS livro (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    titulo          TEXT    NOT NULL,
    isbn            TEXT,
    ano_publicacao  INTEGER,
    total_paginas   INTEGER NOT NULL CHECK (total_paginas > 0),
    sinopse         TEXT,
    capa_url        TEXT,
    origem          TEXT    NOT NULL DEFAULT 'MANUAL'
                    CHECK (origem IN ('MANUAL', 'CATALOGO', 'GOOGLE_BOOKS')),
    id_autor        INTEGER NOT NULL REFERENCES autor(id),
    id_genero       INTEGER NOT NULL REFERENCES genero(id),
    UNIQUE (titulo, id_autor)
);

CREATE TABLE IF NOT EXISTS leitura (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    id_livro      INTEGER NOT NULL UNIQUE REFERENCES livro(id) ON DELETE CASCADE,
    status        TEXT    NOT NULL DEFAULT 'QUERO_LER'
                  CHECK (status IN ('QUERO_LER', 'LENDO', 'LIDO', 'ABANDONADO')),
    pagina_atual  INTEGER NOT NULL DEFAULT 0 CHECK (pagina_atual >= 0),
    data_inicio   TEXT,
    data_fim      TEXT,
    nota          INTEGER CHECK (nota BETWEEN 1 AND 5),
    resenha       TEXT
);

CREATE TABLE IF NOT EXISTS registro_progresso (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    id_leitura     INTEGER NOT NULL REFERENCES leitura(id) ON DELETE CASCADE,
    data_registro  TEXT    NOT NULL,
    pagina_atual   INTEGER NOT NULL,
    paginas_lidas  INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS lista_tematica (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    nome          TEXT NOT NULL UNIQUE,
    descricao     TEXT,
    data_criacao  TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS lista_livro (
    id_lista     INTEGER NOT NULL REFERENCES lista_tematica(id) ON DELETE CASCADE,
    id_livro     INTEGER NOT NULL REFERENCES livro(id) ON DELETE CASCADE,
    data_adicao  TEXT    NOT NULL,
    PRIMARY KEY (id_lista, id_livro)
);

CREATE INDEX IF NOT EXISTS idx_livro_titulo   ON livro(titulo);
CREATE INDEX IF NOT EXISTS idx_livro_autor    ON livro(id_autor);
CREATE INDEX IF NOT EXISTS idx_registro_leit  ON registro_progresso(id_leitura);

-- Visão usada pela tabela interativa (QSqlTableModel) da janela principal.
DROP VIEW IF EXISTS vw_acervo;
CREATE VIEW vw_acervo AS
SELECT l.id                                                           AS id,
       l.titulo                                                       AS titulo,
       a.nome                                                         AS autor,
       g.nome                                                         AS genero,
       l.ano_publicacao                                               AS ano,
       l.isbn                                                         AS isbn,
       l.total_paginas                                                AS paginas,
       r.status                                                       AS status,
       CASE r.status WHEN 'QUERO_LER'  THEN 'Quero Ler'
                     WHEN 'LENDO'      THEN 'Lendo'
                     WHEN 'LIDO'       THEN 'Lido'
                     WHEN 'ABANDONADO' THEN 'Abandonado' END          AS situacao,
       r.pagina_atual                                                 AS pagina_atual,
       ROUND(100.0 * r.pagina_atual / l.total_paginas, 1)             AS progresso,
       CASE WHEN r.nota IS NULL THEN ''
            ELSE substr('★★★★★', 1, r.nota) END                       AS avaliacao
FROM livro l
JOIN autor  a ON a.id = l.id_autor
JOIN genero g ON g.id = l.id_genero
LEFT JOIN leitura r ON r.id_livro = l.id;
