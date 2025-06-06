const token = "46631e72fac07dd8e9ddeb52b3bfda16db88deed"; 
const addressInput = document.getElementById("userAdress");
const suggestionsBox = document.getElementById("suggestions");

addressInput.addEventListener("input", async function () {
  const query = this.value.trim();

  if (query.length < 3) {
    suggestionsBox.innerHTML = "";
    suggestionsBox.style.display = "none";
    return;
  }

  const response = await fetch("https://suggestions.dadata.ru/suggestions/api/4_1/rs/suggest/address", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
      "Accept": "application/json",
      "Authorization": "Token " + token
    },
    body: JSON.stringify({ query })
  });

  const result = await response.json();
  suggestionsBox.innerHTML = "";

  if (result.suggestions.length > 0) {
    suggestionsBox.style.display = "block";
    result.suggestions.forEach(suggestion => {
      const div = document.createElement("div");
      div.classList.add("suggestion");
      div.textContent = suggestion.value;
      div.addEventListener("click", () => {
        addressInput.value = suggestion.value;
        suggestionsBox.innerHTML = "";
        suggestionsBox.style.display = "none"; 
      });
      suggestionsBox.appendChild(div);
    });
  } else {
    suggestionsBox.style.display = "none"; 
  }
});


document.addEventListener("click", (e) => {
  if (!addressInput.contains(e.target) && !suggestionsBox.contains(e.target)) {
    suggestionsBox.innerHTML = "";
    suggestionsBox.style.display = "none";
  }
});

const payStatusElement = document.getElementById("payStatus");
const paymentInputs = document.querySelectorAll('input[name="payment"]');
const paymentOptions = document.querySelectorAll(".payment-option");
let selectedPayment = "Наличные"; 

paymentOptions.forEach(option => {
  option.addEventListener("click", () => {
    paymentOptions.forEach(opt => opt.classList.remove("active"));
    option.classList.add("active");
    selectedPayment = option.getAttribute("data-value");
    payStatusElement.textContent = selectedPayment;
  });
});


function renderSelectedProducts(cart) {
    const container = document.getElementById('cartContainer');
    container.innerHTML = '';

    if (cart.length === 0) {
        container.innerHTML = 'Вы ничего не выбрали.';
        return;
    }

    let totalAmount = 0;
    let totalQuantity = 0;

    cart.forEach(item => {
        const imageUrl = item.image_url || '/static/img/default-image.png';
        const quantity = item.quantity;
        const price = item.price;
        const discountPrice =  item.price - item.discount_price ?? item.price;
        const finalPrice = discountPrice * quantity;
        totalAmount += finalPrice;
        totalQuantity += quantity;

        const card = `
            <div style="display: flex; gap: 25px; margin-bottom: 20px;">
                <div class="product">
                    <div style="display: flex; gap: 40px; align-items: center;">
                        <img src="${imageUrl}" alt="" />
                        <div class="product-information">
                            <h1 style="max-width: 600px;">${item.name}</h1>
                            <h5>Размер: ${item.size || 'N/A'}</h5>
                            <h5>Скидка: ${item.discount_price ? (item.discount_price * quantity).toLocaleString('ru-RU') + "₽" : 'Без скидки'}</h5>
                            <h5>Артикул: ${item.sku || 'N/A'}</h5>
                        </div>
                    </div>
                    <div class="product-another-content" style="align-items: flex-end;">
                        <h1 class="product-cost" data-price="${price}">
                            ${(discountPrice * quantity).toLocaleString('ru-RU')}₽
                            <h5>Количество: ${item.quantity}</h5>
                        </h1>
                    </div>
                </div>
            </div>
        `;

        container.insertAdjacentHTML('beforeend', card);
    });

    const totalPriceElement = document.getElementById("totalPrice");
    if (totalPriceElement) {
        totalPriceElement.textContent = totalAmount.toLocaleString('ru-RU') + " ₽";
    }

    const totalQuantityElement = document.getElementById("totalQuantity");
    if (totalQuantityElement) {
        totalQuantityElement.textContent = totalQuantity.toLocaleString('ru-RU') + " шт.";
    }
}

function loadSelectedProducts(productIds) {
    fetch('/api/checkout/products', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        credentials: 'include',
        body: JSON.stringify({ product_ids: productIds })
    })
    .then(response => response.json())
    .then(data => {
        renderSelectedProducts(data);
    })
    .catch(error => {
        console.error("Ошибка загрузки выбранных товаров:", error);
        document.getElementById('cartContainer').textContent = 'Не удалось загрузить выбранные товары.';
    });
}

document.addEventListener('DOMContentLoaded', () => {
    const storedIds = localStorage.getItem('selectedProductIds');
    if (!storedIds) {
        document.getElementById('cartContainer').textContent = 'Нет выбранных товаров для отображения.';
        return;
    }

    const productIds = JSON.parse(storedIds);
    if (!Array.isArray(productIds) || productIds.length === 0) {
        document.getElementById('cartContainer').textContent = 'Список товаров пуст.';
        return;
    }

    loadSelectedProducts(productIds);
});

paymentOptions.forEach(option => {
  option.addEventListener("click", () => {
    paymentOptions.forEach(opt => opt.classList.remove("active"));
    option.classList.add("active");
    selectedPayment = option.getAttribute("data-value");
    payStatusElement.textContent = selectedPayment;
  });
});

function validateForm() {
    let isValid = true;

    const name = document.getElementById("userName");
    const phone = document.getElementById("userPhone");
    const email = document.getElementById("userEmail");
    const address = document.getElementById("userAdress");

    const nameError = document.getElementById("userNameError");
    const phoneError = document.getElementById("userPhoneError");
    const emailError = document.getElementById("userEmailError");
    const addressError = document.getElementById("userAdressError");

    const nameRegex = /^[А-Яа-яЁё\s]{2,}$/;
    if (!nameRegex.test(name.value.trim())) {
        nameError.style.display = "block";
        isValid = false;
    } else {
        nameError.style.display = "none";
    }

    const phoneRegex = /^(?:\+7|8)\d{10}$/;
    if (!phoneRegex.test(phone.value.trim())) {
        phoneError.style.display = "block";
        isValid = false;
    } else {
        phoneError.style.display = "none";
    }

    const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
    if (!emailRegex.test(email.value.trim())) {
        emailError.style.display = "block";
        isValid = false;
    } else {
        emailError.style.display = "none";
    }

    if (address.value.trim().length < 5) {
        addressError.style.display = "block";
        isValid = false;
    } else {
        addressError.style.display = "none";
    }

    return isValid;
}

document.getElementById("placeOrderBtn").addEventListener("click", async () => {
    if (!validateForm()) {
        return;
    }
    const payment_method = selectedPayment;
    const address = document.getElementById("userAdress").value.trim();
    try {
        const response = await fetch("/api/order/create", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            credentials: "include",
            body: JSON.stringify({
                payment_method: payment_method,
                address: address 
            })
        });

        const result = await response.json();

        if (response.ok) {
            showToast("Заказ успешно оформлен!", "success");
            localStorage.removeItem("selectedProductIds");
            setTimeout(() => {
                window.location.href = "/basket";
            }, 1500);
        } else {
            showToast("Ошибка оформления заказа! Попробуйте позже.", "danger");
        }

    } catch (error) {
        showToast("Ошибка оформления заказа! Попробуйте позже.", "danger");
    }
});
