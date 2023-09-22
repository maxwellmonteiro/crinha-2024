CREATE TABLE IF NOT EXISTS pessoa (
    id uuid CONSTRAINT pk_pessoa PRIMARY KEY,
    txt_apelido character varying(32) CONSTRAINT unq_pessoa UNIQUE,
    dat_nascimento date NOT NULL,
    txt_nome character varying(100) NOT NULL,
    txt_stack character varying(300),
    busca_trgm TEXT GENERATED ALWAYS AS (
        LOWER(txt_nome || txt_apelido || txt_stack)
    ) STORED
);

CREATE EXTENSION PG_TRGM;
CREATE INDEX CONCURRENTLY IF NOT EXISTS idx_pessoa_busca_tgrm ON PESSOA USING GIST (busca_trgm GIST_TRGM_OPS(SIGLEN=64));
