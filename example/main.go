package main

import (
	"fmt"
	"key_value_db/db"
)

func ok(e error) {
	if e != nil {
		panic(e)
	}
}

func main() {

	// initialized data
	var database *db.KeyValueDb = nil
	var err error = nil
	var key string = "shit"

	// construct a database connection
	database, err = db.NewKeyValueDb("localhost:6708")
	ok(err)

	// get a value for a key
	value, err := database.Get(key)
	ok(err)

	// print the value
	fmt.Printf("get(\"%s\") -> %s\n", key, value)

	// close the connection
	database.Close()
}
