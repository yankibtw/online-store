document.querySelectorAll('.header-navigation-text-style li a').forEach(link => {
    link.addEventListener('click', function () {
        document.querySelectorAll('.header-navigation-text-style li a').forEach(el => el.classList.remove('active'));
        this.classList.add('active');
    });
});

document.querySelectorAll('a[href^="#"]').forEach(anchor => {
    anchor.addEventListener("click", function (e) {
        e.preventDefault();
        document.querySelector(this.getAttribute("href")).scrollIntoView({
            behavior: "smooth"
        });
    });
});

function openModal() {
    document.getElementById("modalOverlay").style.display = "flex";
    toggleToRegister();
}

function closeModal() {
    document.getElementById("modalOverlay").style.display = "none";
}

function toggleToLogin() {
    document.getElementById("registerForm").style.display = "none";
    document.getElementById("loginForm").style.display = "block";
    document.getElementById("modalTitle").innerText = "Войти в аккаунт";
    document.getElementById("modalToggleText").innerHTML = 'или <a href="#" id="toggleToRegister">создать новый аккаунт</a>';
    document.getElementById("toggleToRegister").addEventListener('click', function (e) {
        e.preventDefault();
        toggleToRegister();
    });
}

function toggleToRegister() {
    document.getElementById("loginForm").style.display = "none";
    document.getElementById("registerForm").style.display = "block";
    document.getElementById("modalTitle").innerText = "Создать аккаунт";
    document.getElementById("modalToggleText").innerHTML = 'или <a href="#" id="toggleToLogin">войти в существующий</a>';
    document.getElementById("toggleToLogin").addEventListener('click', function (e) {
        e.preventDefault();
        toggleToLogin();
    });
}

function togglePassword(event) {
    const icon = event.target;
    const input = icon.previousElementSibling;
    if (input && input.type === "password") {
        input.type = "text";
    } else if (input) {
        input.type = "password";
    }
}
document.addEventListener("DOMContentLoaded", function () {
    const isAuthenticated = document.cookie.includes("session_id") || localStorage.getItem("isAuthenticated") === "true";
    toggleHeaderElements(isAuthenticated);
});

async function validateForm() {
    const data = {
        firstName: document.getElementById("firstName").value.trim(),
        lastName: document.getElementById("lastName").value.trim(),
        email: document.getElementById("email").value.trim(),
        phone: document.getElementById("phone").value.trim(),
        password: document.getElementById("password").value.trim(),
    };

    let valid = true;

    const nameRegex = /^[А-Яа-яA-Za-z]{2,}$/;
    const emailRegex = /^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$/;
    const phoneRegex = /^(\+7|8)\d{10}$/;

    valid &= toggleError("firstNameError", nameRegex.test(data.firstName));
    valid &= toggleError("lastNameError", nameRegex.test(data.lastName));
    valid &= toggleError("emailError", !data.email || emailRegex.test(data.email));
    valid &= toggleError("phoneError", !data.phone || phoneRegex.test(data.phone));
    valid &= toggleError("passwordError", data.password.length >= 6);

    if (!valid) return;

    try {
        const response = await fetch('/register', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        });

        if (response.ok) {
            const { sessionId } = await response.json();  

            document.cookie = `session_id=${sessionId}; path=/; max-age=2592000`;  
            localStorage.setItem("isAuthenticated", "true");

            showToast("Регистрация прошла успешно!");
            closeModal();
            window.location.reload();

            toggleHeaderElements(true);
        } else {
            const { error } = await response.json();
            showToast("Ошибка при регистрации!");
        }
    } catch (error) {
        showToast("Ошибка при регистрации!");
    }
}

async function loginFormSubmit() {
    const email = document.getElementById("loginEmail").value.trim();
    const password = document.getElementById("loginPassword").value.trim();

    const emailRegex = /^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$/;

    let valid = true;
    valid &= toggleError("loginEmailError", emailRegex.test(email));  
    valid &= toggleError("loginPasswordError", password.length >= 6);  

    if (!valid) return;

    try {
        const response = await fetch('/login', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ email, password })
        });

        if (response.ok) {
            const data = await response.json(); 
            const { sessionId } = data;

            document.cookie = `session_id=${sessionId}; path=/; max-age=2592000`;  
            localStorage.setItem("isAuthenticated", "true");

            toggleHeaderElements(true);
            closeModal();
        } else {
            const { error } = await response.json();

            if (error === "User not found") {
                document.getElementById("loginEmailError").textContent = "Пользователь с таким E-mail не найден.";
                document.getElementById("loginEmailError").style.display = "block"; 
                document.getElementById("loginPasswordError").style.display = "none"; 

                showToast(error || "Ошибка при входе!");
            } else {
                document.getElementById("loginPasswordError").textContent = "Введен неверный пароль.";
                document.getElementById("loginPasswordError").style.display = "block"; 
                document.getElementById("loginEmailError").style.display = "none"; 
            }
        }
    } catch (error) {
        showToast("Ошибка при входе!");
    }
}


function toggleHeaderElements(isLoggedIn) {
    document.getElementById("loginButtonWrapper").style.display = isLoggedIn ? "none" : "block";
    document.getElementById("accountSection").style.display = isLoggedIn ? "block" : "none";
    const menu = document.querySelector(".account-menu");
    if (menu) menu.style.display = "none";
}

function logout() {
    document.cookie = "session_id=; Max-Age=0; Path=/;"; 
    localStorage.removeItem("isAuthenticated");  
    toggleHeaderElements(false); 
    
    fetch('/logout', { method: 'POST' })
        .then(response => response.json())  
        .then(data => {
        })
        .catch(error => {
            console.error("Error during logout:", error);
        });
}

function setActive(element) {
    document.querySelectorAll('.menu-item').forEach(item => item.classList.remove('active'));
    element.classList.add('active');
}

function toggleAccountMenu() {
    const menu = document.querySelector(".account-menu");
    if (menu) {
        menu.style.display = (menu.style.display === "block") ? "none" : "block";
    }
}

function toggleError(id, condition) {
    const el = document.getElementById(id);
    if (condition) {
        el.style.display = "none";
        return true;
    } else {
        el.style.display = "block";
        return false;
    }
}

function showToast(message) {
    const toastBody = document.querySelector("#liveToast .toast-body");
    toastBody.textContent = message;

    const toastElement = document.getElementById("liveToast");
    const toast = bootstrap.Toast.getOrCreateInstance(toastElement);
    toast.show();
}
