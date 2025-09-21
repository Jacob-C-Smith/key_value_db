# key value db

> [Run](#run)

# Docker compose
Start docker compose
```bash
$ docker compose up
```

## Run
Start the key value database server
```bash
$ ./build/key_value_db_server
```

Feed the database some data
``` bash 
$ ./build/key_value_db_client < ./seed/identity.seed
```

Start the HTTP server
```bash
$ cd example ; go run main.go
```

Hit the database with CURL
```bash
$ curl \
  --request GET \
  --url     'http://localhost:3013/get?key=id:user:0' 
```

## HTTP server
The http server supports get and set calls

| verb   | **endpoint** | description                 | query parameter   | form body |
|--------|--------------|-----------------------------|-------------------|-----------|
| `GET`  | `/get`       | Get a value from a key      | **key** = `<key>` |           |
| `POST` | `/set`       | Update or create a property | **key** = `<key>` | `<value>` |
