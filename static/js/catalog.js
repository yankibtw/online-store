function selectSize(element) {
    document.querySelectorAll('.size').forEach(size => size.classList.remove('active'));
    element.classList.add('active');
}

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
    }
    
    function closeModal() {
        document.getElementById("modalOverlay").style.display = "none";
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
                // Можно добавить перенаправление или обновление страницы
                // window.location.href = '/';
            } else {
                const err = await response.text();
                alert("Ошибка регистрации: " + err);
            }
        } catch (error) {
            alert("Ошибка сети: " + error.message);
        }
    }