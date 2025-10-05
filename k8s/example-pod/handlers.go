package main

import (
	"fmt"
	"net/http"
	"strconv"
	"time"

	"github.com/labstack/echo/v4"
)

// Pet handlers

func (ps *PetStore) AddPet(ctx echo.Context) error {
	ps.mu.Lock()
	defer ps.mu.Unlock()
	
	var pet Pet
	if err := ctx.Bind(&pet); err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid pet data")
	}
	
	// Assign new ID if not provided
	if pet.Id == nil {
		pet.Id = &ps.nextPetID
		ps.nextPetID++
	}
	
	// Validate required fields
	if pet.Name == "" {
		return echo.NewHTTPError(http.StatusBadRequest, "Pet name is required")
	}
	if len(pet.PhotoUrls) == 0 {
		return echo.NewHTTPError(http.StatusBadRequest, "At least one photo URL is required")
	}
	
	ps.pets[*pet.Id] = pet
	return ctx.JSON(http.StatusOK, pet)
}

func (ps *PetStore) UpdatePet(ctx echo.Context) error {
	ps.mu.Lock()
	defer ps.mu.Unlock()
	
	var pet Pet
	if err := ctx.Bind(&pet); err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid pet data")
	}
	
	if pet.Id == nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Pet ID is required")
	}
	
	if _, exists := ps.pets[*pet.Id]; !exists {
		return echo.NewHTTPError(http.StatusNotFound, "Pet not found")
	}
	
	ps.pets[*pet.Id] = pet
	return ctx.JSON(http.StatusOK, pet)
}

func (ps *PetStore) FindPetsByStatus(ctx echo.Context, params FindPetsByStatusParams) error {
	ps.mu.RLock()
	defer ps.mu.RUnlock()
	
	var result []Pet
	statusMap := make(map[FindPetsByStatusParamsStatus]bool)
	for _, status := range params.Status {
		statusMap[status] = true
	}
	
	for _, pet := range ps.pets {
		if pet.Status != nil {
			petStatus := FindPetsByStatusParamsStatus(*pet.Status)
			if statusMap[petStatus] {
				result = append(result, pet)
			}
		}
	}
	
	return ctx.JSON(http.StatusOK, result)
}

func (ps *PetStore) FindPetsByTags(ctx echo.Context, params FindPetsByTagsParams) error {
	ps.mu.RLock()
	defer ps.mu.RUnlock()
	
	var result []Pet
	tagMap := make(map[string]bool)
	for _, tag := range params.Tags {
		tagMap[tag] = true
	}
	
	for _, pet := range ps.pets {
		if pet.Tags != nil {
			for _, tag := range *pet.Tags {
				if tag.Name != nil && tagMap[*tag.Name] {
					result = append(result, pet)
					break
				}
			}
		}
	}
	
	return ctx.JSON(http.StatusOK, result)
}

func (ps *PetStore) DeletePet(ctx echo.Context, petId int64, params DeletePetParams) error {
	ps.mu.Lock()
	defer ps.mu.Unlock()
	
	if _, exists := ps.pets[petId]; !exists {
		return echo.NewHTTPError(http.StatusNotFound, "Pet not found")
	}
	
	delete(ps.pets, petId)
	return ctx.NoContent(http.StatusOK)
}

func (ps *PetStore) GetPetById(ctx echo.Context, petId int64) error {
	ps.mu.RLock()
	defer ps.mu.RUnlock()
	
	pet, exists := ps.pets[petId]
	if !exists {
		return echo.NewHTTPError(http.StatusNotFound, "Pet not found")
	}
	
	return ctx.JSON(http.StatusOK, pet)
}

func (ps *PetStore) UpdatePetWithForm(ctx echo.Context, petId int64) error {
	ps.mu.Lock()
	defer ps.mu.Unlock()
	
	pet, exists := ps.pets[petId]
	if !exists {
		return echo.NewHTTPError(http.StatusNotFound, "Pet not found")
	}
	
	name := ctx.FormValue("name")
	status := ctx.FormValue("status")
	
	if name != "" {
		pet.Name = name
	}
	if status != "" {
		petStatus := PetStatus(status)
		pet.Status = &petStatus
	}
	
	ps.pets[petId] = pet
	return ctx.NoContent(http.StatusOK)
}

func (ps *PetStore) UploadFile(ctx echo.Context, petId int64) error {
	ps.mu.RLock()
	defer ps.mu.RUnlock()
	
	if _, exists := ps.pets[petId]; !exists {
		return echo.NewHTTPError(http.StatusNotFound, "Pet not found")
	}
	
	// Handle file upload (simplified)
	file, err := ctx.FormFile("file")
	if err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "No file uploaded")
	}
	
	additionalMetadata := ctx.FormValue("additionalMetadata")
	
	response := ApiResponse{
		Code:    &[]int32{200}[0],
		Type:    &[]string{"success"}[0],
		Message: &[]string{fmt.Sprintf("File %s uploaded successfully. %s", file.Filename, additionalMetadata)}[0],
	}
	
	return ctx.JSON(http.StatusOK, response)
}

// Store handlers

func (ps *PetStore) GetInventory(ctx echo.Context) error {
	ps.mu.RLock()
	defer ps.mu.RUnlock()
	
	inventory := make(map[string]int32)
	
	for _, pet := range ps.pets {
		if pet.Status != nil {
			status := string(*pet.Status)
			inventory[status]++
		}
	}
	
	return ctx.JSON(http.StatusOK, inventory)
}

func (ps *PetStore) PlaceOrder(ctx echo.Context) error {
	ps.mu.Lock()
	defer ps.mu.Unlock()
	
	var order Order
	if err := ctx.Bind(&order); err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid order data")
	}
	
	// Assign new ID if not provided
	if order.Id == nil {
		order.Id = &ps.nextOrderID
		ps.nextOrderID++
	}
	
	// Set default values
	if order.Status == nil {
		status := Placed
		order.Status = &status
	}
	if order.Complete == nil {
		order.Complete = &[]bool{false}[0]
	}
	if order.ShipDate == nil {
		shipDate := time.Now().AddDate(0, 0, 7) // 7 days from now
		order.ShipDate = &shipDate
	}
	
	// Validate that pet exists
	if order.PetId != nil {
		if _, exists := ps.pets[*order.PetId]; !exists {
			return echo.NewHTTPError(http.StatusBadRequest, "Pet not found")
		}
	}
	
	ps.orders[*order.Id] = order
	return ctx.JSON(http.StatusOK, order)
}

func (ps *PetStore) DeleteOrder(ctx echo.Context, orderId string) error {
	ps.mu.Lock()
	defer ps.mu.Unlock()
	
	id, err := strconv.ParseInt(orderId, 10, 64)
	if err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid order ID")
	}
	
	if _, exists := ps.orders[id]; !exists {
		return echo.NewHTTPError(http.StatusNotFound, "Order not found")
	}
	
	delete(ps.orders, id)
	return ctx.NoContent(http.StatusOK)
}

func (ps *PetStore) GetOrderById(ctx echo.Context, orderId int64) error {
	ps.mu.RLock()
	defer ps.mu.RUnlock()
	
	order, exists := ps.orders[orderId]
	if !exists {
		return echo.NewHTTPError(http.StatusNotFound, "Order not found")
	}
	
	return ctx.JSON(http.StatusOK, order)
}

// User handlers

func (ps *PetStore) CreateUser(ctx echo.Context) error {
	ps.mu.Lock()
	defer ps.mu.Unlock()
	
	var user User
	if err := ctx.Bind(&user); err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid user data")
	}
	
	if user.Username == nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Username is required")
	}
	
	if _, exists := ps.users[*user.Username]; exists {
		return echo.NewHTTPError(http.StatusConflict, "User already exists")
	}
	
	ps.users[*user.Username] = user
	return ctx.NoContent(http.StatusOK)
}

func (ps *PetStore) CreateUsersWithArrayInput(ctx echo.Context) error {
	ps.mu.Lock()
	defer ps.mu.Unlock()
	
	var users []User
	if err := ctx.Bind(&users); err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid user array data")
	}
	
	for _, user := range users {
		if user.Username != nil {
			ps.users[*user.Username] = user
		}
	}
	
	return ctx.NoContent(http.StatusOK)
}

func (ps *PetStore) CreateUsersWithListInput(ctx echo.Context) error {
	// Same implementation as array input
	return ps.CreateUsersWithArrayInput(ctx)
}

func (ps *PetStore) LoginUser(ctx echo.Context, params LoginUserParams) error {
	ps.mu.RLock()
	defer ps.mu.RUnlock()
	
	user, exists := ps.users[params.Username]
	if !exists {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid username/password")
	}
	
	if user.Password == nil || *user.Password != params.Password {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid username/password")
	}
	
	// Create a simple session token
	sessionToken := fmt.Sprintf("session_%s_%d", params.Username, time.Now().Unix())
	
	// Set response headers
	ctx.Response().Header().Set("X-Rate-Limit", "5000")
	ctx.Response().Header().Set("X-Expires-After", time.Now().Add(24*time.Hour).Format(time.RFC3339))
	ctx.Response().Header().Set("Set-Cookie", fmt.Sprintf("AUTH_KEY=%s; Path=/; HttpOnly", sessionToken))
	
	return ctx.JSON(http.StatusOK, sessionToken)
}

func (ps *PetStore) LogoutUser(ctx echo.Context) error {
	// Clear the session cookie
	ctx.Response().Header().Set("Set-Cookie", "AUTH_KEY=; Path=/; HttpOnly; Max-Age=0")
	return ctx.NoContent(http.StatusOK)
}

func (ps *PetStore) DeleteUser(ctx echo.Context, username string) error {
	ps.mu.Lock()
	defer ps.mu.Unlock()
	
	if _, exists := ps.users[username]; !exists {
		return echo.NewHTTPError(http.StatusNotFound, "User not found")
	}
	
	delete(ps.users, username)
	return ctx.NoContent(http.StatusOK)
}

func (ps *PetStore) GetUserByName(ctx echo.Context, username string) error {
	ps.mu.RLock()
	defer ps.mu.RUnlock()
	
	user, exists := ps.users[username]
	if !exists {
		return echo.NewHTTPError(http.StatusNotFound, "User not found")
	}
	
	// Don't return password
	user.Password = nil
	
	return ctx.JSON(http.StatusOK, user)
}

func (ps *PetStore) UpdateUser(ctx echo.Context, username string) error {
	ps.mu.Lock()
	defer ps.mu.Unlock()
	
	if _, exists := ps.users[username]; !exists {
		return echo.NewHTTPError(http.StatusNotFound, "User not found")
	}
	
	var updatedUser User
	if err := ctx.Bind(&updatedUser); err != nil {
		return echo.NewHTTPError(http.StatusBadRequest, "Invalid user data")
	}
	
	// Ensure username matches the path parameter
	updatedUser.Username = &username
	
	ps.users[username] = updatedUser
	return ctx.NoContent(http.StatusOK)
}