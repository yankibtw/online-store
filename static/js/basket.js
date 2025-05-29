function increment(event, productId) {
    event.preventDefault();
    const countElement = document.querySelector(`.count[data-id="${productId}"]`);
    let count = parseInt(countElement.textContent);
    if (count < 25){
        count++; 
        countElement.textContent = count;
    
        const cartItem = cart.find(item => item.cart_item_id === productId);
        if (cartItem) {
            cartItem.quantity = count;
        }
    
        updatePrice(productId, count);
        updateCartQuantity(productId, count);
        updateSummary(cart);
        updateCartItemUI(productId, cartItem);
    }
}

function decrement(event, productId) {
    event.preventDefault();
    const countElement = document.querySelector(`.count[data-id="${productId}"]`);
    let count = parseInt(countElement.textContent);
    if (count > 1) { 
        count--; 
        countElement.textContent = count;

        const cartItem = cart.find(item => item.cart_item_id === productId);
        if (cartItem) {
            cartItem.quantity = count;
        }

        updatePrice(productId, count);
        updateCartQuantity(productId, count);
        updateSummary(cart);
        updateCartItemUI(productId, cartItem);
    }
}

function updatePrice(productId, count) {
    const priceElement = document.querySelector(`.count[data-id="${productId}"]`)
        .closest('.product-another-content')
        .querySelector('.product-cost');

    const pricePerItem = parseFloat(priceElement.getAttribute('data-price'));

    const totalPrice = (count * pricePerItem).toFixed(2);
    priceElement.textContent = Number(totalPrice).toLocaleString('ru-RU') + '₽';
}

function updateCartItemUI(productId, cartItem) {
    const priceElement = document.querySelector(`.product[data-id="${productId}"] .product-cost`);
    const countElement = document.querySelector(`.product[data-id="${productId}"] .count`);

    if (priceElement && countElement && cartItem) {
        const pricePerItem = parseFloat(priceElement.getAttribute('data-price'));
        const totalPrice = (cartItem.quantity * pricePerItem).toFixed(2);
        
        priceElement.textContent = Number(totalPrice).toLocaleString('ru-RU') + '₽';
        countElement.textContent = cartItem.quantity;
    }
}

function updateSummary(cart) {
    const checkedCheckboxes = document.querySelectorAll('.item-checkbox:checked');
    const selectedIds = Array.from(checkedCheckboxes).map(cb => parseInt(cb.value));
    const selectedItems = cart.filter(item => selectedIds.includes(item.cart_item_id));

    let totalQuantity = 0;
    let totalFullPrice = 0;
    let totalDiscountedPrice = 0;

    selectedItems.forEach(item => {
        const quantity = item.quantity;
        const price = item.price;
        const discountPrice = item.discount_price ?? item.price;
        const itemTotal = discountPrice * quantity;

        totalQuantity += quantity;
        totalFullPrice += price * quantity;
        totalDiscountedPrice += itemTotal;
    });

    const totalSavings = totalFullPrice - totalDiscountedPrice;

    const summaryHTML = `
        <div class="basket-products-info">
            <h1>Итог:</h1>
            <ul style="display: flex; justify-content: space-between; margin-top: 42px;">
                <ul style="color: #8F8F8F">
                    <li>${totalQuantity} ${getPlural(totalQuantity, ['товар', 'товара', 'товаров'])}</li>
                    <li>Скидка</li>
                </ul>
                <ul style="color: #414141">
                    <li>${totalFullPrice.toLocaleString('ru-RU')}₽</li>
                    <li style="font-weight: 600;">-${totalDiscountedPrice.toLocaleString('ru-RU')}₽</li>
                </ul>
            </ul>
            <hr style="margin: 24px 0">
            <div class="exit-data">
                <h5>Итог:</h5>
                <h4 style="color: #414141">${(totalSavings).toLocaleString('ru-RU')}₽</h4>
            </div>
            <button id="checkoutBtn">Оформить заказ</button>
        </div>
    `;

    const summaryContainer = document.querySelector('.basket-products-info');
    if (summaryContainer) {
        summaryContainer.outerHTML = summaryHTML;
    } else {
        const container = document.getElementById('cartContainer');
        container.insertAdjacentHTML('beforeend', summaryHTML);
    }
    addCheckoutListener();  
}

function loadCart() {
    fetch('/api/cart', {
        method: 'GET',
        credentials: 'include'
    })
    .then(response => response.json())
    .then(data => {
        if (data.status === "empty") {
            document.getElementById('cartContainer').innerHTML = 'Ваша корзина пуста.';
            return;
        }
        cart = data;
        renderCart(cart);
    })
    .catch(error => {
        console.error('Ошибка при загрузке товаров в корзину:', error);
        document.getElementById('cartContainer').innerHTML = 'Не удалось загрузить товары в корзину.';
    });
}

function renderCart(cart) {
    const container = document.getElementById('cartContainer');
    container.innerHTML = '';  

    if (cart.length === 0) {
        container.innerHTML = 'Ваша корзина пуста.';
        return;
    }

    cart.forEach(item => {
        const imageUrl = item.image_url || '/static/img/default-image.png';
        const quantity = item.quantity;
        const price = item.price;
        const discountPrice = item.discount_price ?? item.price;
        const itemTotal = discountPrice * quantity;

const card = `
    <div style="display: flex; gap: 25px">
        <div class="form-check" style="display: flex; justify-content: center; align-items: center;
            margin-bottom: 60px;">
            <input 
                class="form-check-input item-checkbox"
                style="width:27px; height:27px; gap: 10px" 
                type="checkbox" 
                value="${item.cart_item_id}" 
                id="check-${item.cart_item_id}" 
                checked
            >
            <label class="form-check-label" for="check-${item.cart_item_id}"></label>
        </div>
        <div class="product">
            <div style="display: flex; gap: 40px; align-items: center;">
                <img src="${imageUrl}" alt="" style="" />
                <div class="product-information">
                    <h1 style="max-width: 600px;">${item.name}</h1>
                    <h5>Размер: ${item.size || 'N/A'}</h5>
                    <h5>Скидка: ${item.discount_price + "₽" || 'N/A'}</h5>
                    <h5>Артикул: ${item.sku || 'N/A'}</h5>
                </div>
            </div>

            <div class="product-another-content">
                <div class="product-actions">
                    <div class="product-inc">
                        <a href="#" onclick="increment(event, ${item.cart_item_id}); return false;">
                            <img src="/static/img/basket-img/inc.png" alt=""/>
                        </a>
                        <h4 class="count" data-id="${item.cart_item_id}">${item.quantity}</h4>
                        <a href="#" onclick="decrement(event, ${item.cart_item_id}); return false;">
                            <img src="/static/img/basket-img/dec.png" alt=""/>
                        </a>
                    </div>
                    <div>
                        <a href="#" style="display:flex;" onclick="removeFromCart(${item.cart_item_id});">
                            <img src="/static/img/basket-img/Delete.png" alt=""/>
                        </a>
                    </div>
                </div>
                <h1 class="product-cost" data-price="${price}">${(price * quantity).toLocaleString('ru-RU')}₽</h1>
            </div>
        </div>
    </div>
`;
        container.insertAdjacentHTML('beforeend', card);
    });

    updateSummary(cart); 
    document.querySelectorAll('.item-checkbox').forEach(checkbox => {
        checkbox.addEventListener('change', () => {
            updateSummary(cart);
        });
    });
}

function getPlural(n, forms) {
    const mod10 = n % 10;
    const mod100 = n % 100;

    if (mod10 === 1 && mod100 !== 11) return forms[0];
    if (mod10 >= 2 && mod10 <= 4 && (mod100 < 10 || mod100 >= 20)) return forms[1];
    return forms[2];
}

function removeFromCart(productId) {
    fetch(`/api/cart/remove/${productId}`, {
        method: 'POST',
        credentials: 'include'
    })
    .then(response => {
        if (response.ok) {
            loadCart(); 
        } else {
            alert('Не удалось удалить товар из корзины');
        }
    })
    .catch(error => {
        console.error('Ошибка при удалении товара из корзины:', error);
    });
}

function updateCartQuantity(cartItemId, newQuantity) {
    fetch(`/api/cart/update_quantity/${cartItemId}`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        credentials: 'include',
        body: JSON.stringify({ quantity: newQuantity })
    })
    .then(response => {
        if (!response.ok) {
            console.error("Ошибка при обновлении количества в корзине");
        }
    })
    .catch(error => {
        console.error("Ошибка запроса при обновлении количества:", error);
    });
}

document.addEventListener('DOMContentLoaded', () => {
    loadCart();
});

function addCheckoutListener() {
    const checkoutBtn = document.getElementById('checkoutBtn');
    const checkboxes = document.querySelectorAll('.item-checkbox');

    checkboxes.forEach(cb => {
    cb.addEventListener('change', () => {
        const selectedIds = Array.from(document.querySelectorAll('.item-checkbox:checked'))
                                .map(cb => parseInt(cb.value));
        localStorage.setItem('selectedProductIds', JSON.stringify(selectedIds));
    });
    });

    if (checkoutBtn) {
        checkoutBtn.addEventListener('click', (event) => {
            event.preventDefault();

            const checkedCheckboxes = document.querySelectorAll('.item-checkbox:checked');
            const selectedIds = Array.from(checkedCheckboxes).map(cb => parseInt(cb.value));

            if (selectedIds.length === 0) {
                alert('Пожалуйста, выберите хотя бы один товар для оплаты.');
                return;
            }

            fetch('/api/checkout/products', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                credentials: 'include',
                body: JSON.stringify({ product_ids: selectedIds })
            })
            .then(response => {
                if (!response.ok) {
                    throw new Error('Ошибка при получении данных товаров');
                }
                return response.json();
            })
            .then(data => {
                localStorage.setItem('checkoutProducts', JSON.stringify(data));
                window.location.href = '/order';
            })
            .catch((error) => {
                console.error(error);
                alert('Ошибка соединения с сервером.');
            });
        });
    }
}
