#FROM bash:5.2.15
FROM gcc:13.2

WORKDIR /usr/local/rinha-backend

COPY ./rinha ./
COPY ./crinha.conf ./

CMD ["./rinha"]