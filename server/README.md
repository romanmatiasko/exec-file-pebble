# Exec File server

A simple Node.js server for Exec File pebble app. The server requires **Node 
v4.2** or later.

## Setup

```sh
# Clone the repo (or just the server part of it)
cd server
npm install
npm start
```

Executables are located in **scripts/** directory. There are few examples for 
controlling the volume of a Mac, you can replace them or add there your own 
ones. 

Additional arguments can be passed to app.js:

- `port` Specify port on which the server should run. By default, set to 
**3030**.

- `ip` Whitelist IP addresses that can access the server API. It can be single
regex, or list of regexes separated by comma. By default, set to `.*` (any IP).

You can pass the arguments to `npm start` or directly to `./app.js`

Examples:
```sh
npm start port=5000 ip=192.168.1.121
./app.js ip=192.168.0.1,192.168.1.*
```

## Logging

Set env variable `NODE_DEBUG` to **ExecFile** if you want to enable logging to 
console. `npm start` does it automatically, by running 
`NODE_DEBUG=ExecFile ./app.js`.

## API

If you want to provide your own server for Exec File pebble app, the server 
needs to have 2 routes implemented:

**GET /**

Response:
```js
{
  files: [[String filename, String label]]
}
```

Example:
```js
{
  files: [['file1.sh', 'Do something'], ['file2.sh', 'Do something else']]
}
```

**POST /exec**

Payload:
```js
{
  file: String filename
}
```
Response:
```js
{
  stdout: String message,
  stderr: String message
}
```

Example:
```js
// POST /exec
{
  file: 'file1.sh'
}
// returns 200 OK
{
  stdout: 'Success!'
}
// returns 500 Iternal Server Error
{
  stderr: 'Something went wrong'
}
```
