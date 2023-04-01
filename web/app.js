// configuracion básica

var urlint="http://192.168.4.204/";
var urlext="http://ubanov.synology.me:20480/";
var password="";



function getJSON(url,callback)
{
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.responseType = 'json';
	xhr.timeout = 4000;		// timeout 4000ms (4s)
    xhr.onload = function() {
      var status = xhr.status;
      if (status === 200) {
        callback(null, xhr.response);
      } else {
        callback(status, xhr.response);
      }
    };
    // info sobre eventos: https://xhr.spec.whatwg.org/#event-handlers
    xhr.onerror = function() {
	  callback("NETERROR",null);
    };
	xhr.onabort = function() {
	  callback("ABORT",null);
    };
	xhr.ontimeout = function() {
	  callback("TMEOUT",null);
    };
    try {
      xhr.send();
	} catch(e) {
	  callback("Catch",e);	// no lo he visto nunca, pero por si aca (pruebas hasta que he encontrado el onabort)
	}
}


var executingstatusexternal=0;
var executingstatusinternal=0;
var errorstatus=0;

// poner en la imagen del portal la imagen que corresponda
// 0 portal-open.jpg (imagen normal)
// 1 portal-opening.jpg (imagen abriendo)
// 2 portal-openerr.jpg (imagen error acceder)
function changescreen(id)
{
  if(id==0)
    document.getElementById("portal1").src="portal-open.jpg";
  else if(id==1)
    document.getElementById("portal1").src="portal-opening.jpg";
  else
    document.getElementById("portal1").src="portal-openerr.jpg";
}

function changescreen0()
{
  changescreen(0);
}


function openportal()
{
    getJSON(urlext + 'open' + password, function(err,data) {
      if (err !== null || typeof data.resultcode === 'undefined' || data.resultcode !=0 ) {
				// si error
      } else {
	    changescreen(1);		// cambiar a imagen de abriendo (desde checkstatus se podria poner como normal)... 
		setTimeout(changescreen0,data.relaytime); // pero por ser mas exactos en el tiempo  lo hacemos asi
      }
    });

    getJSON(urlint + 'open' + password, function(err,data) {
      if (err !== null || typeof data.resultcode === 'undefined' || data.resultcode !=0 ) {
				// si error
      } else {
	    changescreen(1);		// cambiar a imagen de abriendo (desde checkstatus se podria poner como normal)... 
		setTimeout(changescreen0,data.relaytime); // pero por ser mas exactos en el tiempo  lo hacemos asi
      }
    });
}


function checkstatus()
{
  if(executingstatusexternal==0 && errorstatus!=1) {
    executingstatusexternal=1;

      getJSON(urlext + 'status' + password, function(err,data) {
	    executingstatusexternal=0;
        if (err !== null || typeof data.resultcode === 'undefined' || data.resultcode !=0 ) {
	      errorstatus=errorstatus|1;
		  if(errorstatus==3)
	        changescreen(2);
        } else {
	      errorstatus=errorstatus&2;
          if(data.opening==="1")
	        changescreen(1);
		  else
	        changescreen(0);
        }
      });
  }

  if(executingstatusinternal==0 && errorstatus!=2) {
    executingstatusinternal=1;

      getJSON(urlint + 'status' + password, function(err,data) {
        executingstatusinternal=0;
        if (err !== null || typeof data.resultcode === 'undefined' || data.resultcode !=0 ) {
	      errorstatus=errorstatus|2;
		  if(errorstatus==3)
	        changescreen(2);
        } else {
	      errorstatus=errorstatus&1;
          if(data.opening==="1")
	        changescreen(1);
	  	  else
	        changescreen(0);
        }
      });
  }

  setTimeout(checkstatus,1000);
}


setTimeout(checkstatus,500);

