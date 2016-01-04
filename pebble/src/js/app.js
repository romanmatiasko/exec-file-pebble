void function() {
  let config = null;
  let filesFetched = false;

  function getConfig() {
    return config || {
      ip_address: localStorage.getItem('ip_address'),
      port: localStorage.getItem('port'),
    };
  }

  function setConfig(newConfig) {
    localStorage.setItem('ip_address', newConfig.ip_address);
    localStorage.setItem('port', newConfig.port);
    config = newConfig;
  }

  function parseJSON(xhrResponseText) {
    try {
      return JSON.parse(xhrResponseText);
    } catch (e) {
      return {};
    }
  }

  function parseXHRError(xhr) {
    let error = parseJSON(xhr.responseText).stderr;
    if (!error) {
      error = xhr.responseText
        ? `${xhr.status} ${xhr.responseText}`
        : xhr.statusText;
    }
    return error.substr(0, 320);
  }

  function ajax(method, route, data, callback) {
    const xhr = new XMLHttpRequest();

    xhr.open(method,
      `http://${config.ip_address}:${config.port}${route}`, true);
    if (method === 'POST') {
      xhr.setRequestHeader('Content-Type', 'application/json; charset=utf-8');
    }
    xhr.onreadystatechange = () => {
      if (xhr.readyState !== 4) return;
      if (xhr.status >= 200 && xhr.status < 300) {
        callback(parseJSON(xhr.responseText));
      } else {
        Pebble.sendAppMessage({
          msgErrorKey: !xhr.status
            ? 'Connection error\n\n' +
              `Server at ${config.ip_address}:${config.port} is not available`
            : parseXHRError(xhr)
        });
        console.log(`AJAX_REQUEST_FAIL ${method} ${route} - ${xhr.status}`);
      }
    };
    xhr.timeout = 5000;
    xhr.send(data ? JSON.stringify(data) : null);
  }

  function init() {
    config = getConfig();

    if (!config.ip_address || !config.port) {
      Pebble.sendAppMessage({
        msgErrorKey:
          'Improperly configured\n\nCheck config page on your phone'
      });
      return;
    }

    ajax('GET', '/', null, ({files}) => {
      if (!files || !files.length) {
        Pebble.sendAppMessage({
          msgErrorKey:
            'Improperly configured\n\nNo executable files were found'
        });
        return;
      }

      filesFetched = true;
      files.forEach(([name, label]) => Pebble.sendAppMessage({
        filesLengthKey: files.length,
        fileNameKey: name,
        fileLabelKey: label,
      }));
    });
  }

  Pebble.addEventListener('ready', init);

  Pebble.addEventListener('appmessage', ({payload}) => {
    if (payload.menuItemKey) {
      ajax('POST', '/exec', {file: payload.menuItemKey}, ({stdout}) => {
        Pebble.sendAppMessage({
          msgSuccessKey: (stdout || 'OK').substr(0, 320),
        });
      });
    }
  });

  Pebble.addEventListener('showConfiguration', () => {
    const configEncoded = encodeURIComponent(JSON.stringify(getConfig()));
    const url = 'https://cdn.rawgit.com/romanmatiasko/exec-file-pebble/' +
      'master/config/index.html';
    Pebble.openURL(`${url}?config=${configEncoded}`);
  });

  Pebble.addEventListener('webviewclosed', ({response}) => {
    if (response) {
      setConfig(JSON.parse(decodeURIComponent(response)));
    }
    if (!filesFetched) {
      init();
    }
  });
}();
