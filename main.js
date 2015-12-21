'use strict'
// Importing required libraries
var process = require('process')
var Spark = require('spark')
var bunyan = require('bunyan')
var express = require('express')
var bodyParser = require('body-parser')
var basicAuth = require('basic-auth')
var cluster = require('cluster')
var os = require('os')


var token = process.env.LED_CONTROLLER_TOKEN
var httpPort = process.env.LED_CONTROLLER_PORT || 8080
var authorizedUser = process.env.LED_CONTROLLER_USER || 'user'
var authorizedPass = process.env.LED_CONTROLLER_PASS || 'password'

// Allowed mode and params
var modes = {
  colorWipe: {
    params: ['r','g','b'],
  },
  rainbow: {
    params: null,
  },
  rainbowCycle: {
    params: null,
  },
  fullColorCycle: {
    params: null,
  },
  randomDots: {
    params: null,
  },
  turnedOff: {
    params: null,
  },
}

// Device
var device;

// Logging configuration
var log = bunyan.createLogger({name: 'LedStripController'})

// Define login callback
var loginCallback = (err, body) => {
  if (err) {
    log.error(`API login KO: ${err}`)
  } else {
    log.info('API login OK')
    process()
  }
}

var process = () => {
  Spark.listDevices((err, devices) => {
    device = devices[0];
    log.info(`Got device ${device.attributes.name}`)
  })
}

// Initiate login
Spark.login({accessToken: token}, loginCallback)

// Web server
var app = express()

// Authentication
var auth = (req, res, next) => {
  var unauthorized = (res) => {
    res.set('WWW-Authenticate', 'Basic realm=Authorization Required')
    return res.sendStatus(401)
  }

  var user = basicAuth(req)

  if (!user || !user.name || !user.pass) {
    return unauthorized(res)
  }

  if (user.name === authorizedUser  && user.pass === authorizedPass) {
    return next()
  } else {
    return unauthorized(res)
  }
}

app.use(auth, express.static('static'))
app.use(bodyParser.json())
app.use(bodyParser.urlencoded({extended: true}));

// DRY
var defineAccessor = (endpointName, variableName, setterName) => {
  app.post(`/${endpointName}`, auth, (req, res) => {
    log.info(`Set mode to ${req.body[variableName]} requested`)
    device.callFunction(setterName, req.body[variableName], (err, data) => {
      if (err) {
        log.error(`Set mode to ${req.body[variableName]} KO:`, err)
      } else {
        log.info(`Set mode to ${req.body[variableName]} OK`)
      }
    })
    res.end()
  })

  app.get(`/${endpointName}`, auth, (req, res) => {
    log.info(`Get ${variableName} requested`)
    device.getVariable(variableName, function(err, data) {
      if (err) {
        console.log(`An error occurred while getting ${variableName}: ${err}`)
      } else {
        res.setHeader('Content-Type', 'application/json')
        res.send(data)
        res.end()
      }
    });
  })
}

app.get('/modes', auth, (req, res) => {
  log.info('Get modes requested')
  res.setHeader('Content-Type', 'application/json');
  res.send(modes)
  res.end()
})

defineAccessor('mode', 'mode', 'setMode')
defineAccessor('wait', 'wait', 'setWait')
defineAccessor('power', 'power', 'setPower')

if (cluster.isMaster) {
  var numWorkers = os.cpus().length
  log.info('Master cluster setting up ' + numWorkers + ' workers...')

  for (var i = 0; i < numWorkers; i++) {
    cluster.fork()
  }

  cluster.on('online', (worker) => {
    log.info('Worker ' + worker.process.pid + ' is online');
  });

  cluster.on('exit', (worker, code, signal) => {
    log.info(`Worker ${worker.process.pid} died with code: ${code}, and signal: ${signal}`);
    log.info('Starting a new worker');
    cluster.fork();
  });

} else {
  app.listen(httpPort, () => {
    log.info('Express listening on', httpPort)
  })
}
