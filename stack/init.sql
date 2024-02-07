drop table transacao;
drop table cliente;

create table cliente (
    id integer constraint pk_cliente primary key,
    limite bigint not null,
    saldo bigint not null
);

create table transacao (
    id uuid constraint pk_transacao primary key,
    id_cliente integer references cliente(id),
    valor integer not null,
    tipo char not null,
    descricao character varying (10) not null,
    realizada_em timestamp not null default current_timestamp
);

insert into cliente(id, limite, saldo) values(1, 100000, 0);
insert into cliente(id, limite, saldo) values(2, 80000, 0);
insert into cliente(id, limite, saldo) values(3, 1000000, 0);
insert into cliente(id, limite, saldo) values(4, 10000000, 0);
insert into cliente(id, limite, saldo) values(5, 500000, 0);

