# log
echo "[docker] Starting key value db node on $(hostname -I | awk '{print $1}')"

./build/key_value_db_server 

# log
echo "[docker] key value db node up"