
history.pushState(null, '', location.href);

window.onpopstate = function(event) {
    location.reload();
};
