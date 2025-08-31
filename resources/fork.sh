# log
echo "[docker] Starting key value db node on $(hostname -I | awk '{print $1}')"

./build/key_value_db_server & sleep 1.0;
./build/key_value_db_client < seed.commands & sleep 1;
cd example && go run main.go; cd -

# log
echo "[docker] key value db node up"