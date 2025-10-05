//go:build tools
// +build tools

//go:generate go run github.com/oapi-codegen/oapi-codegen/v2/cmd/oapi-codegen -generate server -package main -o server-gen.go openapi/api-spec.yaml
//go:generate go run github.com/oapi-codegen/oapi-codegen/v2/cmd/oapi-codegen -generate client,types -package main -o client-gen.go openapi/api-spec.yaml
package main

import (
	"fmt"
	"log"
	"net/http"
	"strconv"
	"sync"
	"time"

	"github.com/labstack/echo/v4"
	"github.com/labstack/echo/v4/middleware"
)

// PetStore implements the ServerInterface for the generated server
type PetStore struct {
	mu    sync.RWMutex
	pets  map[int64]Pet
	users map[string]User
	orders map[int64]Order
	nextPetID int64
	nextOrderID int64
}

// NewPetStore creates a new PetStore instance with some sample data
func NewPetStore() *PetStore {
	store := &PetStore{
		pets:   make(map[int64]Pet),
		users:  make(map[string]User),
		orders: make(map[int64]Order),
		nextPetID: 1,
		nextOrderID: 1,
	}
	
	// Add some sample data
	store.addSampleData()
	return store
}

func (ps *PetStore) addSampleData() {
	// Sample pets
	pets := []Pet{
		{
			Id:   &[]int64{1}[0],
			Name: "Buddy",
			Category: &Category{
				Id:   &[]int64{1}[0],
				Name: &[]string{"Dogs"}[0],
			},
			PhotoUrls: []string{"https://example.com/buddy.jpg"},
			Status:    &[]PetStatus{PetStatusAvailable}[0],
			Tags: &[]Tag{
				{Id: &[]int64{1}[0], Name: &[]string{"friendly"}[0]},
			},
		},
		{
			Id:   &[]int64{2}[0],
			Name: "Mittens",
			Category: &Category{
				Id:   &[]int64{2}[0],
				Name: &[]string{"Cats"}[0],
			},
			PhotoUrls: []string{"https://example.com/mittens.jpg"},
			Status:    &[]PetStatus{PetStatusPending}[0],
			Tags: &[]Tag{
				{Id: &[]int64{2}[0], Name: &[]string{"quiet"}[0]},
			},
		},
	}
	
	for _, pet := range pets {
		ps.pets[*pet.Id] = pet
		if *pet.Id >= ps.nextPetID {
			ps.nextPetID = *pet.Id + 1
		}
	}
	
	// Sample users
	users := []User{
		{
			Id:        &[]int64{1}[0],
			Username:  &[]string{"john_doe"}[0],
			FirstName: &[]string{"John"}[0],
			LastName:  &[]string{"Doe"}[0],
			Email:     &[]string{"john@example.com"}[0],
			Password:  &[]string{"password123"}[0],
			Phone:     &[]string{"555-1234"}[0],
			UserStatus: &[]int32{1}[0],
		},
	}
	
	for _, user := range users {
		ps.users[*user.Username] = user
	}
}

func main() {
	store := NewPetStore()
	
	e := echo.New()
	
	// Middleware
	e.Use(middleware.Logger())
	e.Use(middleware.Recover())
	e.Use(middleware.CORS())
	
	// Register our handlers
	RegisterHandlers(e, store)
	
	log.Println("Starting Petstore server on :8080")
	log.Fatal(e.Start(":8080"))
}