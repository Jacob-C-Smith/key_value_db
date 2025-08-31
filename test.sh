#!/bin/bash

# Author: Jacob Smith 
#
# Commentary: This script starts N timestamp server containers 
#      and records each IP address in servers.txt

##########################################
# Check for valid command line arguments #
##########################################
case $# in 
3) ;;
*) echo "Usage: " `basename $0` " <server-quantity> <api-port> <http-port>"; exit 1;;
esac

########################
# Function definitions #
########################

# Kill the server and the RMI registry
function cleanup ( )
{
    
    # Log
    printf "${CYAN}╔═════════════════╗\n║ Stopping tester ║\n╚═════════════════╝${RESET}\n\n"

    # Kill the server
    kill $RMIPID &> /dev/null

    # Log 
    printf "${YELLOW}╭──────────────────────╮\n│ Servers are still up │\n╰──────────────────────╯${RESET}\n\n"
}

# Catch signals
trap cleanup SIGINT;

####################
# Initialized data #
####################
EXIT_SUCCESS=0
EXIT_FAILURE=1
SERVER_QUANTITY=$1
REDIS_PORT=5110
API_PORT=$2
HTTP_PORT=$3
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[0;93m'
RESET='\033[0m'
declare -a SERVER_ADDRESSES

######################
# Prepare the tester #
######################

# Log
printf "${CYAN}╔══════════════════════════════════╗\n║ Requesting superuser permissions ║\n╚══════════════════════════════════╝${RESET}\n"

# Ask for superuser up front
sudo ls > /dev/null

# Kill any running docker containers (if any)
if [ -n "$(sudo docker ps -q)" ]; then
    sudo docker kill $(sudo docker ps -q) > /dev/null
fi

# Remove any leftover key value db containers (stopped or running)
if [ -n "$(sudo docker ps -aq --filter name=key_value_db_)" ]; then
    sudo docker rm -f $(sudo docker ps -aq --filter name=key_value_db_) > /dev/null
fi

# Clear the old log files
cd logs; rm -f *; cd ..
rm -f servers.txt
touch servers.txt

# Log
printf "\n${CYAN}╔═════════════════╗\n║ Starting tester ║\n╚═════════════════╝${RESET}\n\n"

# Compile everything
make clean > /dev/null
make > logs/make_output.txt 2> logs/make_output.txt

# Catch compile errors
if [ $? != $EXIT_SUCCESS ]; then

    # Print an error to standard out
    printf "${RED}[Error: Failed to compile project!]\nMake says: ${RESET}\n\n"

    # Concatenate makefile output to standard out
    cat logs/make_output.txt

    # Clean up
    cleanup

    # Error
    exit $EXIT_FAILURE
else

    # Print an error to standard out
    printf "${GREEN}╭───────────────────────────────╮\n│ Project compiled successfully │\n╰───────────────────────────────╯${RESET}\n\n"
fi

# Construct a list of host ports for each instance of the key value db server (macOS can't reach container IPs)
# We'll publish container port API_PORT (e.g., 6710) to host ports API_PORT+i and point the client to 127.0.0.1:hostPort
for (( i=0; i < $SERVER_QUANTITY; i++ ))
do
    HOST_PORT=$(($API_PORT + $i))
    SERVER_ADDRESSES[i]="127.0.0.1"
    echo "${SERVER_ADDRESSES[i]}:${HOST_PORT}" >> servers.txt
    echo "" > "logs/key_value_db_${i}.txt"
done

# Build the docker image
sudo docker build -t key_value_db . > logs/docker_build_output.txt 2> logs/docker_build_output.txt

# Start key value db (publish unique host ports for each container)
for (( i=0; i < $SERVER_QUANTITY; i++ ))
do
    CONTAINER_NAME="key_value_db_${i}"
    HOST_PORT=$(($API_PORT + $i))
    printf "${GREEN}Starting ${CONTAINER_NAME} on 127.0.0.1:${HOST_PORT} -> container :${API_PORT}${RESET}\r"
    CID=$(sudo docker run -d --name "${CONTAINER_NAME}" \
        -p "${HOST_PORT}:${API_PORT}/udp" \
        -p "${HOST_PORT}:${API_PORT}/tcp" \
        -p "3013:${HTTP_PORT}/udp" \
        -p "3013:${HTTP_PORT}/tcp" \
        key_value_db)
    sudo docker logs --details -f "${CID}" > "logs/${CONTAINER_NAME}.txt" 2>&1 &
done

printf "\n${GREEN}All ${SERVER_QUANTITY} server are up and running${RESET}\n\n"

for (( i=0; i < $SERVER_QUANTITY; i++ ))
do
    HOST_PORT=$(($API_PORT + $i))
    echo "${SERVER_ADDRESSES[i]}:${HOST_PORT}"
done

# Clean up
cleanup

# Success
exit $EXIT_SUCCESS