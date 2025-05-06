function increment(event, productId) {
    event.preventDefault();
    const countElement = document.querySelector(`.count[data-id="${productId}"]`);
    let count = parseInt(countElement.textContent);
    count++;
    countElement.textContent = count;
    updatePrice(productId, count);
}

function decrement(event, productId) {
    event.preventDefault();
    const countElement = document.querySelector(`.count[data-id="${productId}"]`);
    let count = parseInt(countElement.textContent);
    if (count > 1) {
        count--;
        countElement.textContent = count;
        updatePrice(productId, count);
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

function loadCart() {
    fetch('/api/cart', {
        method: 'GET',
        credentials: 'include'
    })
    .then(response => response.json())
    .then(data => {
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

    if (cart.length > 0) {
        cart.forEach(item => {
            const imageUrl = item.image_url
                ? item.image_url
                : '/static/img/default-image.png';

                const totalPrice = (item.price * item.quantity).toFixed(2);

                const card = `
                <div class="product">
                    <div style="display: flex; gap: 40px">
                        <img src="${imageUrl}" alt="">
                        <div class="product-information">
                            <h1 style="max-width: 600px;">${item.name}</h1>
                            <h5>Размер: ${item.size || 'N/A'}</h5>
                        </div>
                    </div>
                    <div class="product-another-content">
                        <div class="product-actions">
                            <div class="product-inc">
                                <a href="#" onclick="increment(event, ${item.id}); return false;">
                                    <img src="/static/img/basket-img/inc.png" alt="">
                                </a>
                                <h4 class="count" data-id="${item.id}">${item.quantity}</h4>
                                <a href="#" onclick="decrement(event, ${item.id}); return false;">
                                    <img src="/static/img/basket-img/dec.png" alt="">
                                </a>
                            </div>
                            <div>
                                <a href="#" onclick="removeFromCart(${item.id});">
                                    <img src="/static/img/basket-img/Delete.png" alt="">
                                </a>
                            </div>
                        </div>
                        <h1 class="product-cost" data-price="${item.price}">${Number(totalPrice).toLocaleString('ru-RU')}₽</h1>
                    </div>
                </div>
                `;                

            container.insertAdjacentHTML('beforeend', card);
        });
    } else {
        container.innerHTML = 'Ваша корзина пуста.';
    }
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

document.addEventListener('DOMContentLoaded', () => {
    loadCart();
});
