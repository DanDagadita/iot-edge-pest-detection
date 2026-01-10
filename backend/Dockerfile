FROM golang:1.25-alpine AS builder

WORKDIR /app

COPY go.mod go.sum ./
RUN go mod download

COPY . .
RUN CGO_ENABLED=0 GOOS=linux go build -ldflags="-w -s" -o /usr/local/bin/app ./cmd/app/

FROM alpine:latest

RUN addgroup -g 1000 appuser \
    && adduser -u 1000 -G appuser -D appuser

USER appuser

COPY --from=builder --chown=appuser:appuser /usr/local/bin/app /app/

CMD ["/app/app"]
