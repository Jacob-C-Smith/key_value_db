HOST="localhost"
PORT="3013"
GET_ENDPOINT="http://${HOST}:${PORT}/get"

export GET_ORG="http://${HOST}:${PORT}/get?key=id:org"
export GET_USER="http://${HOST}:${PORT}/get?key=id:user"
export GET_ROLE="http://${HOST}:${PORT}/get?key=id:role"
export GET_GROUP="http://${HOST}:${PORT}/get?key=id:group"

function GetOrg ( )
{
    echo "org:$1 ->" $(curl --request GET --url "${GET_ORG}:$1") >> out
    echo "" >> out
}

function GetUser ( )
{
    echo "user:$1        ->" $(curl --request GET --url "${GET_USER}:$1") >> out
    echo "user:$1:org    ->" $(curl --request GET --url "${GET_USER}:$1:org") >> out
    echo "user:$1:groups ->" $(curl --request GET --url "${GET_USER}:$1:groups") >> out
    echo "user:$1:roles  ->" $(curl --request GET --url "${GET_USER}:$1:roles") >> out
    echo "" >> out
}

function GetRole ( )
{
    echo "role:$1             ->" $(curl --request GET --url "${GET_ROLE}:$1") >> out
    echo "role:$1:org         ->" $(curl --request GET --url "${GET_ROLE}:$1:org") >> out
    echo "role:$1:permissions ->" $(curl --request GET --url "${GET_ROLE}:$1:permissions") >> out
    echo "" >> out
}

function GetGroup ( )
{
    echo "group:$1       ->" $(curl --request GET --url "${GET_GROUP}:$1") >> out
    echo "group:$1:org   ->" $(curl --request GET --url "${GET_GROUP}:$1:org") >> out
    echo "group:$1:roles ->" $(curl --request GET --url "${GET_GROUP}:$1:roles") >> out
    echo "group:$1:users ->" $(curl --request GET --url "${GET_GROUP}:$1:users") >> out
    echo "" >> out
}

# clear output file
cat /dev/null > out

# Get orgs
GetOrg 0

# Get users
for (( i=0; i<7; i++ )); do GetUser $i; done

# Get roles
for (( i=0; i<6; i++ )); do GetRole $i; done

# Get groups
for (( i=0; i<4; i++ )); do GetGroup $i; done
