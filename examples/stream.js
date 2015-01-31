'use strict';

var zoom = require('../lib');

zoom.connection('192.83.186.170:210/INNOPAC')
  .set('preferredRecordSyntax', 'usmarc')
  .query('prefix', '@attr 1=4 台灣')
  .createReadStream()
  .on('data', function (record) {
    console.log(record.json);
  });