function loadFavorites() {
    fetch('/api/favorites', {
        method: 'GET',
        credentials: 'include'
    })
    .then(response => response.json())
    .then(data => {
        favorites = data;
        renderFavorites(favorites); 
    })
    .catch(error => {
        console.error('Ошибка при загрузке избранных товаров:', error);
        document.getElementById('favoritesContainer').innerHTML = 'Не удалось загрузить избранные товары.';
    });
}

function renderFavorites(favorites) {
    const container = document.getElementById('favoritesContainer');
    container.innerHTML = ''; 

    if (favorites.length > 0) {
        favorites.forEach(product => {
            const imageUrl = product.image_url
                ? `${product.image_url}`
                : '/static/img/default-image.png';

            const card = `
                <div class="card" data-name="${product.name.toLowerCase()}" data-brand="${product.brand.toLowerCase()}">
                    <div class="card-img-style">
                        <img src="${imageUrl}" alt="${product.name}">
                    </div>
                    <hr style="margin: 0;">
                    <div class="card-description">
                        <div class="card-titles">
                            <h1>${product.name}</h1>
                            <h3>${product.brand}</h3>
                            <h2>${product.price.toFixed(2)} ₽</h2>
                        </div>
                        <button class="accept-card-btn" onclick="window.location.href='/card?id=${product.id}'">Подробнее</button>
                        <button class="accept-card-btn" onclick="removeFromFavorites(${product.id})">Удалить из избранного</button>
                    </div>
                </div>
            `;

            container.insertAdjacentHTML('beforeend', card);
        });
    } else {
        container.innerHTML = 'У вас нет избранных товаров.';
    }
}

function removeFromFavorites(productId) {
    fetch(`/api/favorites/remove/${productId}`, {
        method: 'POST',
        credentials: 'include'
    })
    .then(response => {
        if (response.ok) {
            loadFavorites(); 
            showToast("Товар успешно удален из избранного.", "success");
        } else {
            showToast("Не удалось удалить товар из избранного.", "danger");
        }
    })
    .catch(error => {
        console.error('Ошибка при удалении товара из избранного:', error);
    });
}

document.addEventListener('DOMContentLoaded', () => {
    loadFavorites();
});
