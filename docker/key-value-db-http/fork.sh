# log
echo "[docker] Starting http server node on $(hostname -I | awk '{print $1}')"

cd example ; sleep 3.0 ; go run main.go

# log
echo "[docker] http server node up"