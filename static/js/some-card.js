let allProducts = [6];

function loadProducts() {
    fetch('/api/products')
        .then(response => response.json())
        .then(products => {
            allProducts = products;
            updateView(); 
            setupSearch();
            setupSorting();
            setupPriceFilter();
        })
        .catch(error => {
            console.error('Ошибка загрузки продуктов:', error);
        });
}

function renderProducts(products) {
    const container = document.getElementById('catalog');
    container.innerHTML = ''; 

    products.forEach(product => {
        if (!product.name || !product.price) return;

        const imageUrl = product.image_url
            ? `${product.image_url}`
            : '/static/img/default-image.png';

        const card = `
            <div class="some-card" data-name="${product.name.toLowerCase()}" data-brand="${product.brand.toLowerCase()}">
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
                </div>
            </div>
        `;

        container.insertAdjacentHTML('beforeend', card);
    });
}

document.addEventListener('DOMContentLoaded', loadProducts);
