"use strict"
// Importing required libraries
var process = require('process')
var Spark = require('spark')
var bunyan = require('bunyan')
var express = require('express')
var bodyParser = require('body-parser')
var basicAuth = require('basic-auth')

var token = process.env.CONTROLLER_TOKEN
var httpPort = process.env.CONTROLLER_PORT
var authorizedUser = process.env.CONTROLLER_USER
var authorizedPass = process.env.CONTROLLER_PASS

// Allowed mode and params
var modes = {
  'colorWipe': {
    'params': ['r','g','b']
  },
  'rainbow': {
    'params': null
  },
  'rainbowCycle': {
    'params': null
  },
  'fullColorCycle': {
    'params': null
  },
  'randomDots': {
    'params': null
  },
  'turnedOff': {
    'params': null
  },
}

// Device
var device;

// Logging configuration
var log = bunyan.createLogger({name: 'LedStripController'})

// Define login callback
var loginCallback = (err, body) => {
  if (err) {
    log.error('API login KO: ', err)
  } else {
    log.info('API login OK')
    process()
  }
}

var process = () => {
  Spark.listDevices((err, devices) => {
    device = devices[0];
    log.info("Got device", device.attributes.name)
  })
}

// Initiate login
Spark.login({accessToken: token}, loginCallback)

// Web server
var app = express()
app.use(express.static('static'))
app.use(bodyParser.json())
app.use(bodyParser.urlencoded({extended: true}));

// Authentication
var auth = (req, res, next) => {
  var unauthorized = (res) => {
    res.set('WWW-Authenticate', 'Basic realm=Authorization Required')
    return res.send(401)
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

// Handler definitions

app.get('/modes', auth, (req, res) => {
  log.info("Get modes requested")
  res.setHeader('Content-Type', 'application/json');
  res.send(modes)
  res.end()
})
app.post('/mode', auth, (req, res) => {
  log.info("Set mode to", req.body.mode, "requested")
  if(req.body.mode in modes) {
    device.callFunction("setMode", req.body.mode, (err, data) => {
      if (err) {
        log.error("Set mode to", req.body.mode, "KO:", err)
      } else {
        log.info("Set mode to", req.body.mode, "OK")
      }
    })
  } else {
    log.error("Mode", req.body.mode, "not found")
  }
  res.end()
})

app.get('/mode', auth, (req, res) => {
  log.info("Get mode requested")
  device.getVariable('mode', function(err, data) {
    if (err) {
      console.log('An error occurred while getting mode:', err);
    } else {
      res.setHeader('Content-Type', 'application/json');
      res.send(data)
      res.end()
    }
  });
})

app.post('/wait', auth, function (req, res) {
  log.info("Set wait to", req.body.wait, "requested")
  device.callFunction("setWait", req.body.wait, (err, data) => {
    if (err) {
      log.error("Set wait to", req.body.wait, "KO:", err)
    } else {
      log.info("Set wait to", req.body.wait, "OK")
    }
  })
  res.end()
})

app.get('/wait', auth, function (req, res) {
  log.info("Get wait requested")
  device.getVariable('wait', function(err, data) {
  if (err) {
    console.log('An error occurred while getting wait:', err);
  } else {
    res.setHeader('Content-Type', 'application/json');
    res.send(data)
    res.end()
  }
});
})

app.listen(httpPort, () => {
  log.info('Express listening on', httpPort)
})
