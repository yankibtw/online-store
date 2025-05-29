function showContent(tab) {
    const description = document.getElementById('product-description');
    const reviews = document.getElementById('reviews');
    if (description) description.style.display = tab === 'description' ? 'block' : 'none';
    if (reviews) reviews.style.display = tab === 'reviews' ? 'block' : 'none';

    const descTab = document.getElementById('desc-tab');
    const reviewsTab = document.getElementById('reviews-tab');
    if (descTab) descTab.classList.toggle('active', tab === 'description');
    if (reviewsTab) reviewsTab.classList.toggle('active', tab === 'reviews');
}

const images = ["review-test.jpg", "review-test.jpg", "review-test.jpg", "review-test.jpg", "review-test.jpg"];
const imageContainer = document.getElementById("imageContainer");

if (imageContainer) {
    images.slice(0, 3).forEach(src => {
        let img = document.createElement("img");
        img.src = "../static/img/card-img/" + src;
        img.alt = "Product image";
        imageContainer.appendChild(img);
    });

    if (images.length > 3) {
        let morePhotos = document.createElement("div");
        morePhotos.classList.add("more-photos");
        morePhotos.textContent = `+${images.length - 3} фото`;
        imageContainer.appendChild(morePhotos);
    }
}

const toastTrigger = document.getElementById('liveToastBtn');
const toastLiveExample = document.getElementById('liveToast');

if (toastTrigger && toastLiveExample) {
    const toastBootstrap = bootstrap.Toast.getOrCreateInstance(toastLiveExample);
    toastTrigger.addEventListener('click', () => {
        toastBootstrap.show();
    });
}

document.addEventListener("DOMContentLoaded", function () {
    const productId = new URLSearchParams(window.location.search).get("id");
    if (!productId) return;

    const reviewsContainer = document.getElementById("reviews");
    if (!reviewsContainer) return;

    fetch(`/api/product/${productId}`)
        .then(response => {
            if (!response.ok) throw new Error("Товар не найден");
            return response.json();
        })
        .then(product => {
            renderProduct(product);
            return fetch(`/api/reviews/${productId}`);
        })
        .then(res => {
            if (!res.ok) throw new Error("Ошибка загрузки отзывов");
            return res.json();
        })
        .then(data => {
            renderReviews(data.reviews || []);
        })
        .catch(err => {
            console.error(err);
            reviewsContainer.innerHTML = `
                <div class="add-review">
                    <p>У данного товара нет отзывов. Станьте первым, кто оставил отзыв об этом товаре!</p>
                    <button id="add-review-btn">Написать отзыв</button>
                </div>`;
        });

    function showMessage(msg) {
        reviewsContainer.innerHTML = `<p>${msg}</p>`;
    }

    function renderReviews(reviews = []) {
        if (!Array.isArray(reviews) || reviews.length === 0) {
            reviewsContainer.innerHTML = `
                <div class="add-review">
                    <p>У данного товара нет отзывов. Станьте первым, кто оставил отзыв об этом товаре!</p>
                    <button id="add-review-btn">Написать отзыв</button>
                </div>`;
            return;
        }

        reviewsContainer.innerHTML = reviews.map(renderReview).join('');
    }

    function renderReview({ userName, reviewText, selectedSize, rating, reviewDate, images = [] }) {
        const avatar = images[0] || "../static/img/card-img/review.png";
        const stars = "★".repeat(rating);

        const imageBlock = images.length
            ? `<div class="images">${images.map(url => `<img src="${url}" alt="Изображение отзыва">`).join('')}</div>`
            : '';

        return `
            <div class="review">
                <div class="user">
                    <img src="${avatar}" alt="User">
                    <div class="user-info">
                        <strong>${userName}</strong>
                        <div class="review-mark">
                            <p>Размер: ${selectedSize || "Не указан"}</p>
                            <div style="display: flex; gap: 30px">
                                <div class="stars">${stars}</div>
                                <p>${reviewDate}</p>
                            </div>
                        </div>
                    </div>
                </div>
                <p>${reviewText}</p>
                ${imageBlock}
            </div>`;
    }

    function renderProduct(product) {
        const nameBr = document.getElementById("product-name-br");
        const name = document.getElementById("product-name");
        const brand = document.getElementById("product-brand");
        const sku = document.getElementById("sku-txt");
        const image = document.getElementById("product-image");
        const price = document.getElementById("product-price");
        const discPrice = document.getElementById("product-disc-price");
        const description = document.getElementById("product-description");

        if (nameBr) nameBr.textContent = product.name;
        if (name) name.textContent = product.name;
        if (brand) brand.textContent = product.brand;
        if (sku) sku.textContent = product.sku;
        if (image) image.src = product.image_url;
        if (price) price.textContent = (product.price - product.discount_price) + " ₽";
        if (discPrice) discPrice.textContent = product.price + " ₽";
        if (description) description.textContent = product.description;

        const carouselInner = document.querySelector("#carouselExampleIndicators .carousel-inner");
        const carouselIndicators = document.querySelector("#carouselExampleIndicators .carousel-indicators");
        if (!carouselInner || !carouselIndicators) return;

        carouselInner.innerHTML = '';
        carouselIndicators.innerHTML = '';

        product.images.forEach((img, index) => {
            const activeClass = img.is_main ? 'active' : '';
            const indicator = document.createElement('button');
            indicator.type = 'button';
            indicator.setAttribute('data-bs-target', '#carouselExampleIndicators');
            indicator.setAttribute('data-bs-slide-to', index);
            indicator.className = activeClass;
            indicator.setAttribute('aria-label', `Slide ${index + 1}`);
            if (activeClass) indicator.setAttribute('aria-current', 'true');
            carouselIndicators.appendChild(indicator);

            const div = document.createElement('div');
            div.className = `carousel-item ${activeClass}`;
            const imgTag = document.createElement('img');
            imgTag.src = img.url;
            imgTag.className = 'd-block w-100';
            imgTag.alt = product.name;
            div.appendChild(imgTag);
            carouselInner.appendChild(div);
        });
    }

    document.querySelectorAll(".add-to-favourite").forEach(button => {
        button.addEventListener("click", async function () {
            try {
                const response = await fetch(`/api/favorites/add/${productId}`, {
                    method: "POST",
                    credentials: "include",
                });

                const result = await response.json();

                if (response.ok) {
                    alert("Товар добавлен в избранное!");
                } else {
                    alert("Войдите в аккаунт, чтобы добавить товар в избранное!");
                }
            } catch (err) {
                alert("Ошибка сети");
                console.error(err);
            }
        });
    });

    const addToCartBtn = document.getElementById("add-to-cart-btn");
    if (addToCartBtn) {
        addToCartBtn.addEventListener("click", async function () {
            const selectedSize = document.querySelector('input[name="size_option"]:checked');
            if (!selectedSize) {
                alert("Пожалуйста, выберите размер перед добавлением в корзину.");
                return;
            }

            try {
                const response = await fetch(`/api/cart/add/${productId}`, {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    credentials: "include",
                    body: JSON.stringify({ size: selectedSize.value })
                });

                const result = await response.json();
                if (response.ok) {
                    alert(result.message || "Товар успешно добавлен в корзину!");
                } else {
                    alert(result.error || "Не удалось добавить товар в корзину.");
                }
            } catch (err) {
                console.error(err);
                alert("Ошибка при добавлении в корзину.");
            }
        });
    }

    document.body.addEventListener("click", function(event) {
        if (event.target && event.target.id === "add-review-btn") {
            const reviewFormContainer = document.getElementById("review-form-container");
            if (reviewFormContainer) {
                reviewFormContainer.style.display = "block";
                window.scrollTo({
                    top: reviewFormContainer.offsetTop,
                    behavior: 'smooth'
                });
            } else {
                console.warn('Элемент #review-form-container не найден в DOM');
            }
        }
    });

    const cancelBtn = document.getElementById("cancel-review");
    if (cancelBtn) {
        cancelBtn.addEventListener("click", function () {
            const formContainer = document.getElementById("review-form-container");
            if (formContainer) {
                formContainer.style.display = "none";
            }
        });
    }

    const reviewForm = document.getElementById("review-form");
    if (reviewForm) {
        reviewForm.addEventListener("submit", async function (e) {
            e.preventDefault();

            const formData = new FormData(e.target);
            const rating = formData.get("rating");
            const comment = formData.get("comment").trim();
            const selected_size = formData.get("selected_size").trim();
            const image_url = formData.get("image_url").trim();

            if (!rating || !comment) {
                alert("Пожалуйста, укажите рейтинг и комментарий.");
                return;
            }

            const image_urls = image_url ? [image_url] : [];

            try {
                const response = await fetch(`/api/reviews/add/${productId}`, {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    credentials: "include",
                    body: JSON.stringify({
                        rating: parseInt(rating),
                        comment,
                        selected_size,
                        image_urls
                    }),
                });

                const result = await response.json();

                if (response.ok) {
                    alert(result.message || "Отзыв успешно добавлен!");
                    document.getElementById("review-form-container").style.display = "none";
                    e.target.reset();

                    fetch(`/api/reviews/${productId}`)
                        .then(res => res.json())
                        .then(data => {
                            renderReviews(data.reviews);
                        });
                } else {
                    alert(result.error || "Ошибка при добавлении отзыва.");
                }
            } catch (err) {
                console.error(err);
                alert("Ошибка сети при отправке отзыва.");
            }
        });
    }
});