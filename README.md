# Route Planner
## Summary
This project is based on a coding assignment I once received as part of a job technical test. Assignment.pdf is a sanitised version of this original technical test, and provides a detailed description of what the assignment entailed.  
<br/><br/>
The high level summary is to provide a client and server application, where the client communicates via tcp to the server in order to request a route calculation between two known locations.
<br/><br/>
It makes use of conan and cmake (via pip packages) as a build toolchain (see conanfile.txt and requirements.txt)

## Usage
The server is started as follows:<br/>
`server [PORT NUMBER] [LOCATION DB FILE] [ROUTE DB FILE]`<br/>
<br/>
where:
- `PORT NUMBER` - Is the port number to listen to inbound connections from the client.
- `LOCATION DB FILE` - This is a path to the locations db file which is a csv file that is a list in the form of "LOCATION NAME, COST", see config/locations.dat for an example
- `ROUTE DB FILE` - This is a path to the routes db file which is a csv file that is a list in the form of "START LOCATION, END LOCATIONS*", see config/routes.dat for an example

The client is started as follows:
<br/>
`client [PORT NUMBER]`<br/>
<br/>
where:
- `PORT NUMBER` - Is the port number of the server to attempt to connect to

Once the client is connected it will list the avaliable locations, from there you can pick the start and end location via its index. The request will be sent to the server to calculate the route cost

## Implementation Notes
In addition to the requrements of the original assignment, I set myself the following aims/requirements:
* Use Conan to manage the dependencies
* Use Cmake as the build system
* Use the boost::asio library for the tcp communcation (having never previously used it)
* Make messaging structure easy to extend so that adding new messages is as trivial and painless as possible.
* Make the code needed to start a tcp client/server and handle the messages as trivial and painless as possible.
* Make use of logging to allow for easier debugging, and understanding of what the application is doing
* Allow for future growth of data sources for the location/route db
* Ensure there is a good level of unit tests for the low level code at a minimum.

## Future Work
The current version represents a basic working version of the route planner, however future changes I'm considering on making:
* Use conan to package up the client/server binaries and the config associated with them, currently everything just gets dumped in the build folder
* Make the location/route db sync happen outside of the client route request code. This has the potential for the client/server to go out of sync with regards to what locations they know about. It would be a good idea to have this happen whenever the data in the sources has actully changed
* Drop use of index's in the route request message.
* Allow the client to add a new location to the location/route db
* Allow the client to edit new location in the location/route db
