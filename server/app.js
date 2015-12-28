#!/usr/bin/env node --harmony

'use strict';

const fs = require('fs');
const cp = require('child_process');
const log = require('util').debuglog('ExecFile');
const app = require('koa')();
const router = require('koa-router')();
const parse = require('co-body');

const rmExtension = str => str.substr(0, str.lastIndexOf('.')) || str;
const capitalize = str => str.charAt(0).toUpperCase() + str.slice(1);
const humanize = str => capitalize(rmExtension(str)).replace(/(_|-)/g, ' ');
const onError = thrw => err => (log(err), thrw(500, 'Internal server error'));

let appPort = 3030;
let ipWhitelist = [/.*/];
let availableFiles = new Set();

process.argv.slice(2).forEach(arg => {
  const portMatched = arg.match(/^port=(\d+)$/);
  const ipMatched = arg.match(/^ip=(.*)$/);

  if (portMatched) {
    appPort = portMatched[1];
  } else if (ipMatched) {
    ipWhitelist = ipMatched[1].split(',').map(ip => new RegExp(ip));
    log('IP whitelist: ', ipWhitelist);
  }
});

router
  .get('/', function *() {
    yield new Promise((resolve, reject) => {
      fs.readdir(`${__dirname}/scripts`, (err, files) => {
        if (err) return reject(err);

        this.body = {
          files: files.map(file => [file, humanize(file)]),
        };
        resolve(files);
      });
    }).then(files => availableFiles = new Set(files))
      .catch(onError(this.throw));
  })
  .post('/exec', function *() {
    const file = this.request.body.file;

    if (!availableFiles.has(file)) {
      this.throw(400, `Unknown file: ${file}`);
    }

    yield new Promise(resolve => {
      cp.execFile(`${__dirname}/scripts/${file}`, (err, stdout, stderr) => {
        this.response.code = err ? 500 : 200;
        this.response.body = {
          stdout: stdout.toString(),
          stderr: stderr.toString(),
        };
        resolve();
      });
    }).catch(onError(this.throw));
  });

app
  .use(function *(next) {
    const startDate = new Date();
    yield next;
    log('%s: %s %s - %dms %j', this.ip, this.method, this.url,
        new Date() - startDate, this.request.body);
  })
  .use(function *(next) {
    if (ipWhitelist.some(ipRgx => ipRgx.test(this.ip))) {
      yield next;
    } else {
      this.throw(403, 'Forbidden');
    }
  })
  .use(function *(next) {
    if (this.is('application/json')) {
      this.request.body = yield parse.json(this);
    } else if (this.method === 'POST') {
      this.throw(415, 'Unsupported Content Type');
    }
    yield next;
  })
  .use(router.routes());

app.on('error', err => log('Server error: ', err));
app.listen(appPort, () => log('Server listening at localhost:%d', appPort));
