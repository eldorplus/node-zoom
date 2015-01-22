NODE-ZOOM
=========

## Requirements

* [gnutls](http://www.gnutls.org/)
* [gcrypt](http://www.gnu.org/software/libgcrypt/)
* [libxml2](http://xmlsoft.org/)

### Debian/Ubuntu

    $ sudo apt-get install libgcrypt11-dev libgnutls-dev libxml2-dev

## Installaction

    $ npm i node-zoom

## Example

    var zoom = require('node-zoom');
    var conn = new zoom.Connection('192.83.186.170:210/INNOPAC');
   
    conn.connect(function (err) {
      if (err) {
        return console.error(err);
      }
      var query = new zoom.Query('@attr 1=4 台灣'); // or new zoom.Query('prefix', '@attr 1=4 台灣')
      conn.search(query, function (err, resultset) {
        var first = resultset.record(1);
        console.log(first.xml());
      });
    });

