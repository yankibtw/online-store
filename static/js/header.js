document.querySelectorAll('.header-navigation-text-style li a').forEach(link => {
    link.addEventListener('click', function() {
        document.querySelectorAll('.header-navigation-text-style li a').forEach(el => {
            el.classList.remove('active');
        });
        
        this.classList.add('active');
        });
    });
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener("click", function(e) {
            e.preventDefault();
            document.querySelector(this.getAttribute("href")).scrollIntoView({
                behavior: "smooth"
            });
        });
    });
    
function openModal() {
    document.getElementById("modalOverlay").style.display = "flex";
    document.getElementById("registerForm").style.display = "block";
    document.getElementById("loginForm").style.display = "none";
    document.getElementById("modalTitle").innerText = "Создать аккаунт";
    document.getElementById("modalToggleText").innerHTML = 'или <a href="#" id="toggleToLogin">войти в существующий</a>';

    document.getElementById("toggleToLogin").addEventListener('click', function(e) {
        e.preventDefault();
        toggleToLogin();
    });
}

function closeModal() {
    document.getElementById("modalOverlay").style.display = "none";
}

function toggleToLogin() {
    document.getElementById("registerForm").style.display = "none";
    document.getElementById("loginForm").style.display = "block";
    document.getElementById("modalTitle").innerText = "Войти в аккаунт";
    document.getElementById("modalToggleText").innerHTML = 'или <a href="#" id="toggleToRegister">создать новый аккаунт</a>';

    document.getElementById("toggleToRegister").addEventListener('click', function(e) {
        e.preventDefault();
        toggleToRegister();
    });
}

function toggleToRegister() {
    document.getElementById("loginForm").style.display = "none";
    document.getElementById("registerForm").style.display = "block";
    document.getElementById("modalTitle").innerText = "Создать аккаунт";
    document.getElementById("modalToggleText").innerHTML = 'или <a href="#" id="toggleToLogin">войти в существующий</a>';

    document.getElementById("toggleToLogin").addEventListener('click', function(e) {
        e.preventDefault();
        toggleToLogin();
    });
}

function togglePassword() {
    let passwordField = document.getElementById("password");
    passwordField.type = passwordField.type === "password" ? "text" : "password";
}

async function validateForm() {
    const data = {
        firstName: document.getElementById("firstName").value.trim(),
        lastName: document.getElementById("lastName").value.trim(),
        email: document.getElementById("email").value.trim(),
        phone: document.getElementById("phone").value.trim(),
        password: document.getElementById("password").value.trim(),
    };

    let valid = true;

    let nameRegex = /^[А-Яа-яA-Za-z]{2,}$/;
    let emailRegex = /^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$/;
    let phoneRegex = /^(\+7|8)\d{10}$/;
    let passwordMinLength = 6;

    if (!nameRegex.test(data.firstName)) {
        document.getElementById("firstNameError").style.display = "block";
        valid = false;
    } else {
        document.getElementById("firstNameError").style.display = "none";
    }

    if (!nameRegex.test(data.lastName)) {
        document.getElementById("lastNameError").style.display = "block";
        valid = false;
    } else {
        document.getElementById("lastNameError").style.display = "none";
    }

    if (data.email && !emailRegex.test(data.email)) {
        document.getElementById("emailError").style.display = "block";
        valid = false;
    } else {
        document.getElementById("emailError").style.display = "none";
    }

    if (data.phone && !phoneRegex.test(data.phone)) {
        document.getElementById("phoneError").style.display = "block";
        valid = false;
    } else {
        document.getElementById("phoneError").style.display = "none";
    }

    if (data.password.length < passwordMinLength) {
        document.getElementById("passwordError").style.display = "block";
        valid = false;
    } else {
        document.getElementById("passwordError").style.display = "none";
    }

    if (!valid) return;

    try {
        const response = await fetch('/register', {
            method: 'POST',
            headers: { 
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(data)
        });

        if (response.ok) {
            const result = await response.text();
            alert(result);
            closeModal();
        } else {
            const err = await response.text();
            alert("Ошибка регистрации: " + err);
        }
    } catch (error) {
        alert("Ошибка сети: " + error.message);
    }
}

async function loginFormSubmit() {
    const data = {
        email: document.getElementById("loginEmail").value.trim(),
        password: document.getElementById("loginPassword").value.trim(),
    };

    let valid = true;

    let emailRegex = /^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$/;
    let passwordMinLength = 6;

    if (data.email && !emailRegex.test(data.email)) {
        document.getElementById("loginEmailError").style.display = "block";
        valid = false;
    } else {
        document.getElementById("loginEmailError").style.display = "none";
    }

    if (data.password.length < passwordMinLength) {
        document.getElementById("loginPasswordError").style.display = "block";
        valid = false;
    } else {
        document.getElementById("loginPasswordError").style.display = "none";
    }

    if (!valid) return;

    try {
        const response = await fetch('/login', {
            method: 'POST',
            headers: { 
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(data)
        });

        if (response.ok) {
            const result = await response.text();
            alert(result);
            closeModal();
        } else {
            const err = await response.text();
            alert("Ошибка авторизации: " + err);
        }
    } catch (error) {
        alert("Ошибка сети: " + error.message);
    }
}

    