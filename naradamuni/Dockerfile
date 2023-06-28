FROM rust:1.69.0-alpine3.16 as build
RUN mkdir /app && apk add --no-cache musl-dev
ADD . /app
WORKDIR /app
RUN cd /app && cargo build --release

FROM alpine:3.16
RUN mkdir /app
COPY --from=build /app/target/release/naradamuni /app/
WORKDIR /app
ENTRYPOINT [ "./naradamuni"]
EXPOSE 8080