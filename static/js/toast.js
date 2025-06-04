function showToast(message, type = "success") {
    const toastEl = document.getElementById("universalToast");
    const toastBody = document.getElementById("universalToastBody");

    if (!toastEl || !toastBody) return;

    toastBody.textContent = message;

    toastEl.classList.remove("bg-success", "bg-danger", "bg-warning", "bg-info");
    toastEl.classList.add(`bg-${type}`);

    const toast = bootstrap.Toast.getOrCreateInstance(toastEl);
    toast.show();
}
