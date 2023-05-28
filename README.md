#MersulTrenurilor

## Introduction
-This project involves implementing a multi-user client that sends commands to a server (signal - receive), which then executes these commands and returns a character string based on the request made by the client.
-The "Train Schedule" application is a server/client type application that provides or updates real-time information from all registered clients about train schedules. Registered users have access to two types of commands:
-Requests for train schedules in the following formats:
  -Trains running that day.
  -Trains departing in the next hour (according to schedule, delayed by x minutes).
  -Based on arrivals in the next hour (according to schedule, delayed by x minutes, x minutes early).
  -Commands to send information to the server about possible delays or if the train arrives/departs early.
## Technologies Used
-The project uses a concurrent TCP server. TCP is a protocol used in applications that require confirmation of data receipt, keeping the order and integrity of the data.
-Concurrency is ensured by using threads, as this is a quick and efficient way to divide processes for clients. Information about the trains is stored in individual XML files for each train route, with tags such as: <Id> for the train id, <Departure> for the departure city, <Arrival> for the arrival city, <Departure_Date> for the train departure date, <Delay> to inform users that train x has a delay of y minutes, etc.
-The application also includes a user database and a registration system, as only users with an account on the platform have access to the main commands. SQLITE, a relational database management system, was used to create this system.

## Application Architecture
-The application design involves the following concepts: server, client, database, and XML files. The server receives connection requests from multiple clients, which it handles concurrently. Depending on the command sent, the server uses the database to check if the user using the client is logged in to an account and the XML files to extract the necessary information for the corresponding response to the command.
-SQLITE is used to manage the user database. The database, users.db, includes the following fields: ID, Name, Password, Status.
-For reading and modifying the XML files, the application uses the libxml/parser.h and limxml/tree.h libraries.
-There are various server functions to handle client requests, such as getInfoToday, getDepartures, getArrivals, setDepartureDelay, setArrivalDelay, setEarlyDeparture, setEarlyArrival, getDepartureCity, getArrivalCity, etc.
-These functions ensure that the application can handle any request related to train schedules, from getting current information about train departures and arrivals to updating information about delays and early arrivals or departures.
  
### Prerequisites
Ensure you have the following installed on your local development machine:
  
  - C++ compiler
  - SQLite3 Library
  - Libxml2 Library
  - POSIX Threads Library (pthread)
  
### Installation
  
  1. Clone the repository to your local machine
  2. Navigate to the project directory
  3. Build the project using the Makefile provided
  4. Run the server program in the background, and then run the client program


