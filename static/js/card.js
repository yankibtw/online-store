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
                    <button id="add-review-btn" data-bs-toggle="modal" data-bs-target="#reviewModal">Написать отзыв</button>
                </div>`;
        });

    function showMessage(msg) {
        reviewsContainer.innerHTML = `<p>${msg}</p>`;
    }
    
    function renderReviews(reviews = []) {
        const hasReviews = Array.isArray(reviews) && reviews.length > 0;

        let html = '';

        if (hasReviews) {
            html += reviews.map(renderReview).join('');
        } else {
            html += `<p>У данного товара нет отзывов. Станьте первым, кто оставил отзыв об этом товаре!</p>`;
        }
        html += `
            <div class="add-review" style="margin-top: 20px;">
                <button id="add-review-btn" class="btn btn-outline-primary" data-bs-toggle="modal" data-bs-target="#reviewModal">
                    Написать отзыв
                </button>
            </div>`;

        reviewsContainer.innerHTML = html;

        const mark = document.getElementById("mark-txt");

        if (mark) {
            if (reviews.length == 0) {
                mark.textContent = "Нет отзывов";
            } else {
                const totalRating = reviews.reduce((sum, r) => sum + r.rating, 0);
                const avgRating = (totalRating / reviews.length).toFixed(1);
                mark.textContent = `${reviews.length} отзыв(ов), средняя оценка: ${avgRating} ★`;
            }
        }


        document.querySelectorAll(".review-image").forEach(img => {
            img.addEventListener("click", () => {
                const modal = document.getElementById("imageModal");
                const modalImg = document.getElementById("modalImage");
                modal.style.display = "flex";
                modalImg.src = img.src;
            });
        });
    }

    function renderReview({ userName, reviewText, selectedSize, rating, reviewDate, images = [] }) {
        const avatar = "../static/img/card-img/user.png";
        const stars = "★".repeat(rating);

        const imageBlock = images.length
            ? `<div class="images">${images.map(url => `<img src="${url}" alt="Изображение отзыва" class="review-image">`).join('')}</div>`
            : '';

        return `
            <div class="review">
                <div class="user">
                    <img src="${avatar}" alt="User">
                    <div class="user-info">
                        <h5>${userName}</h5>
                        <div class="review-mark">
                            <p>Размер: ${selectedSize || "Не указан"}</p>
                            <div style="display: flex; gap: 30px">
                                <div class="stars">${stars}</div>
                                <p>${reviewDate}</p>
                            </div>
                        </div>
                    </div>
                </div>
                <p style="margin-top: 25px;">${reviewText}</p>
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
                    showToast("Товар добавлен в избранное!", "success");
                } else {
                    showToast("Войдите в аккаунт, чтобы добавить товар в избранное!", "danger");
                }
            } catch (err) {
                showToast("Ошибка сети!", "danger");
            }
        });
    });

    const addToCartBtn = document.getElementById("add-to-cart-btn");
    if (addToCartBtn) {
        addToCartBtn.addEventListener("click", async function () {
            const selectedSize = document.querySelector('input[name="size_option"]:checked');
            if (!selectedSize) {
                showToast("Пожалуйста, выберите размер перед добавлением в корзину.", "info");
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
                    showToast("Товар успешно добавлен в корзину!", "success");
                } else {
                    showToast("Не удалось добавить товар в корзину!", "danger");
                }
            } catch (err) {
                showToast("Ошибка при добавлении в корзину.", "danger");
            }
        });
    }

    const modal = document.getElementById("imageModal");
    const modalImg = document.getElementById("modalImage");
    const closeBtn = document.querySelector(".close-btn");

    if (closeBtn && modal) {
        closeBtn.onclick = () => modal.style.display = "none";

        modal.onclick = (e) => {
            if (e.target === modal) {
                modal.style.display = "none";
            }
        };
    }
});

document.getElementById('copyLinkBtn').addEventListener('click', () => {
  const productLink = window.location.href;
  navigator.clipboard.writeText(productLink).then(() => {
    showToast("Ссылка скопирована в буфер обмена", "success");
  }).catch(err => {
    showToast("Не удалось скопировать ссылку", "danger");
  });
});

