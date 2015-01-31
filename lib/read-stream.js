'use strict';

var util = require('util');
var Readable = require('stream').Readable;
var Record = require('./record');

module.exports = ReadStream;

util.inherits(ReadStream, Readable);

var stream = ReadStream.prototype;

function ReadStream(conn, options) {
  options || (options = {});
  options.objectMode = true;
  Readable.call(this, options);

  this._zoomState = {
    conn: conn,
    index: options.index | 0,
    size: (options.size || 20) | 0,
    total: 0,
    resultset: null,
    records: null,
    _waiting: false,
    _destroyed: false
  };
}

stream._read = function () {
  var state = this._zoomState;

  if (state._destroyed) {
    return;
  }

  if (!state.conn._connected) {
    this._connect();
    return state._waiting = true;
  }

  if (!state.resultset) {
    this._getResultset();
    return state._waiting = true;
  }

  if (!(state.records && state.records.hasNext())) {
    this._moreRecords();
    return state._waiting = true;
  }

  var record = state.records.next();
  this.push(record && new Record(record));

  if (++state.index >= state.total) {
    this.push(null);
    this.destroy();
  }
};

stream.destroy = function () {
  var state = this._zoomState;
  state._destroyed = true;
  delete this._zoomState;
  this.emit('close');
};

stream._getResultset = function () {
  var state = this._zoomState;
  var conn = state.conn;

  conn._conn.search(state.conn._query, function (err, resultset) {
    if (err) {
      this.emit('error', err);
      this.destroy();
      return;
    }
    state.resultset = resultset;
    state.total = resultset.size();
    this._zoomReady();
  }.bind(this));
};

stream._moreRecords = function () {
  var state = this._zoomState;
  var resultset = state.resultset;

  resultset.getRecords(state.index, state.size, function (err, records) {
    if (err) {
      this.emit('error', err);
      this.destroy();
      return;
    }
    state.records = records;
    this._zoomReady();
  }.bind(this));
};

stream._zoomReady = function () {
  var state = this._zoomState;

  if (state._waiting && !state._destroyed) {
    state._waiting = false;
    this._read();
  }
};

stream._connect = function () {
  var state = this._zoomState;

  state.conn.connect(function (err) {
    if (err) {
      this.emit('error', err);
      this.destroy();
      return;
    }
    this._zoomReady();
  }.bind(this));
};
