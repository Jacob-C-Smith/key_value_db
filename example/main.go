// package declaration
package main

// imports
import (
	"fmt"
	"key_value_db/db"
	"net/http"
	"os"
)

// data
var database *db.KeyValueDb = nil

// function definitions
func ok(e error) {
	if e != nil {
		panic(e)
	}
}

func database_get(w http.ResponseWriter, r *http.Request) {

	// initialized data
	var err error = nil
	var keys []string = nil
	var key string = ""
	var found bool = false
	var value []byte

	// error check
	if r.Method != "GET" {
		http.Error(w, "Invalid request method", http.StatusMethodNotAllowed)
		return
	}

	// get the key
	keys, found = r.URL.Query()["key"]
	if !found || len(keys[0]) < 1 {
		http.Error(w, "Missing key parameter", http.StatusBadRequest)
		return
	}
	key = keys[0]

	fmt.Printf("getting key \"%s\"\n", key)
	// get a value for a key
	value, err = database.Get(key)
	ok(err)

	// content is json
	w.Header().Set("Content-Type", "application/json")

	// print the value
	fmt.Fprintf(w, "%s", value)
}

func database_set(w http.ResponseWriter, r *http.Request) {

	// initialized data
	var keys []string = nil
	var key string = ""
	var value string = ""
	var found bool = false
	var err error = nil

	// error check
	if r.Method != "POST" {
		http.Error(w, "Invalid request method", http.StatusMethodNotAllowed)
		return
	}

	// get the key
	keys, found = r.URL.Query()["key"]
	if !found || len(keys[0]) < 1 {
		http.Error(w, "Missing key parameter", http.StatusBadRequest)
		return
	}
	key = keys[0]

	// get the value
	// read the value from the body
	valueBytes := make([]byte, r.ContentLength)
	r.Body.Read(valueBytes)

	value = string(valueBytes)

	fmt.Printf("setting key \"%s\" to value \"%s\"\n", key, value)

	// TODO
	resp, err := database.Set(key, string(value))
	ok(err)

	// content is json
	w.Header().Set("Content-Type", "application/json")

	// print the value
	fmt.Fprintf(w, "%s", resp)
}

func main() {

	// initialized data
	var addr string = os.Getenv("ADDR")
	var err error = nil

	// log
	fmt.Printf("Connecting to key value database on %s\n", addr)

	// construct a database connection
	database, err = db.NewKeyValueDb(addr)
	ok(err)

	// log
	fmt.Printf("Listening for http requests on :3013\n")

	// start the server
	http.HandleFunc("/get", database_get)
	http.HandleFunc("/set", database_set)
	http.ListenAndServe(":3013", nil)

	// close the connection
	database.Close()
}
