function loadProducts(){
        fetch('/api/products')
        .then(response => response.json())
        .then(products => {
        const container = document.getElementById('catalog');
        products.forEach(product => {
            const card = `
                <div class="card">
                    <div class="card-img-style"><img src="/static/img/main-img/${product.image_url}" alt="${product.name}"></div>
                    <hr style="margin: 0;">
                    <div class="card-description">
                        <div class="card-titles">
                            <h1>${product.name}</h1>
                            <h3>${product.brand}</h3>
                            <h2>${product.price.toFixed(2)} ₽</h2>
                        </div>
                        <button class="accept-card-btn" onclick="window.location.href='/card'">Подробнее</button>
                    </div>
                </div>
            `;
            container.insertAdjacentHTML('beforeend', card);
        });
        })
        .catch(error => {
        console.error('Ошибка загрузки продуктов:', error);
    });
}

loadProducts();