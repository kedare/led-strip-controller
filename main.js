'use strict'
// Importing required libraries
const process = require('process')
const Spark = require('spark')
const bunyan = require('bunyan')
const express = require('express')
const bodyParser = require('body-parser')
const basicAuth = require('basic-auth')
const cluster = require('cluster')
const os = require('os')


const token = process.env.LED_CONTROLLER_TOKEN
const httpPort = process.env.PORT || 8080
const authorizedUser = process.env.LED_CONTROLLER_USER || 'user'
const authorizedPass = process.env.LED_CONTROLLER_PASS || 'password'

// Allowed mode and params
const modes = {
  steadyColor: {
    params: null,
  },
  colorWipe: {
    params: null,
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
  frozen: {
    params: null,
  },
  turnedOff: {
    params: null,
  },
}

// Device
var device;

// Logging configuration
const log = bunyan.createLogger({name: 'LedStripController'})

// Define login callback
const loginCallback = (err, body) => {
  if (err) {
    log.error(`API login KO: ${err}`)
  } else {
    log.info('API login OK')
    findDevice()
  }
}

const findDevice = () => {
  Spark.listDevices((err, devices) => {
    device = devices[0];
    log.info(`Got device ${device.attributes.name}`)
  })
}

// Initiate login
Spark.login({accessToken: token}, loginCallback)

// Web server
const app = express()

// Authentication
const auth = (req, res, next) => {
  const unauthorized = (res) => {
    res.set('WWW-Authenticate', 'Basic realm=Authorization Required')
    return res.sendStatus(401)
  }

  let user = basicAuth(req)

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
const defineAccessor = (endpointName, variableName, setterName) => {
  app.post(`/${endpointName}`, auth, (req, res) => {
    log.info(`Set ${variableName} to ${req.body[variableName]} requested`)
    device.callFunction(setterName, req.body[variableName], (err, data) => {
      if (err) {
        log.error(`Set ${variableName} to ${req.body[variableName]} KO:`, err)
      } else {
        log.info(`Set ${variableName} to ${req.body[variableName]} OK`)
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
defineAccessor('color', 'color', 'setColor')

app.listen(httpPort, () => {
  log.info('Express listening on', httpPort)
})
