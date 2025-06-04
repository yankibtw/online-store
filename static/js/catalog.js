let currentPage = 1;
const productsPerPage = 6;
let filteredProducts = null;

function setupSearch() {
    const searchInput = document.getElementById('searchInput');
    searchInput.addEventListener('input', function () {
        const query = searchInput.value.toLowerCase().trim();

        const filtered = allProducts.filter(product => {
            const name = product.name.toLowerCase();
            const brand = product.brand.toLowerCase();

            const matches = name.split(' ').some(word => word.startsWith(query)) ||
                            brand.split(' ').some(word => word.startsWith(query));

            return matches;
        });

        currentPage = 1;
        updateView(filtered);
    });
}      

function setupSorting() {
    document.querySelectorAll('#sort-menu .dropdown-item').forEach(item => {
        item.addEventListener('click', (e) => {
            e.preventDefault();
            const sortType = item.getAttribute('data-sort');
            
            const start = (currentPage - 1) * productsPerPage;
            const end = start + productsPerPage;

            let sorted = [...allProducts];

            switch (sortType) {
                case 'price-asc':
                    sorted.sort((a, b) => a.price - b.price);
                    break;
                case 'price-desc':
                    sorted.sort((a, b) => b.price - a.price);
                    break;
                case 'newest':
                    sorted.sort((a, b) => b.id - a.id);
                    break;
            }

            const productsOnPage = sorted.slice(start, end);

            renderProducts(productsOnPage);

            const totalPages = Math.ceil(sorted.length / productsPerPage);
            renderPagination(totalPages);
        });
    });
}

function setupPriceFilter() {
    const minInput = document.getElementById('minPrice');
    const maxInput = document.getElementById('maxPrice');
    const filterBtn = document.getElementById('priceFilterBtn');

    if (!minInput || !maxInput || !filterBtn) return;

    filterBtn.addEventListener('click', () => {
        const min = parseFloat(minInput.value) || 0;
        const max = parseFloat(maxInput.value) || Infinity;

        filteredByPrice = allProducts.filter(product => {
            return product.price >= min && product.price <= max;
        });

        
        currentPage = 1;
        updateView(filteredByPrice);  
    });
}

function setupSorting() {
    document.querySelectorAll('#sort-menu .dropdown-item').forEach(item => {
        item.addEventListener('click', (e) => {
            e.preventDefault();
            const sortType = item.getAttribute('data-sort');

            const start = (currentPage - 1) * productsPerPage;
            const end = start + productsPerPage;

            let productsOnPage = allProducts.slice(start, end);

            switch (sortType) {
                case 'price-asc':
                    productsOnPage.sort((a, b) => a.price - b.price);
                    break;
                case 'price-desc':
                    productsOnPage.sort((a, b) => b.price - a.price);
                    break;
                case 'newest':
                    productsOnPage.sort((a, b) => b.id - a.id);
                    break;
            }

            renderProducts(productsOnPage);

            const totalPages = Math.ceil(allProducts.length / productsPerPage);
            renderPagination(totalPages);
        });
    });
}


function renderPagination(totalPages) {
    const paginationContainer = document.getElementById('pagination');
    const pageLinksContainer = document.querySelector('#pagination');
    
    let pageLinks = '';
    for (let i = 1; i <= totalPages; i++) {
        pageLinks += `<li class="page-item" id="page${i}"><a class="page-link" href="#">${i}</a></li>`;
    }

    pageLinksContainer.innerHTML = `
        <li class="page-item" id="prevPage">
            <a class="page-link" href="#" aria-label="Previous">
                <span aria-hidden="true">&laquo;</span>
            </a>
        </li>
        ${pageLinks}
        <li class="page-item" id="nextPage">
            <a class="page-link" href="#" aria-label="Next">
                <span aria-hidden="true">&raquo;</span>
            </a>
        </li>
    `;

    for (let i = 1; i <= totalPages; i++) {
        const pageLink = document.getElementById(`page${i}`);
        pageLink.addEventListener('click', () => {
            currentPage = i;
            updateView();
        });
    }

    const prevPageBtn = document.getElementById('prevPage');
    prevPageBtn.addEventListener('click', () => {
        if (currentPage > 1) {
            currentPage--;
            updateView();
        }
    });

    const nextPageBtn = document.getElementById('nextPage');
    nextPageBtn.addEventListener('click', () => {
        if (currentPage < totalPages) {
            currentPage++;
            updateView();
        }
    });
}

function updateView(products = null) {
    const productsToUse = products || allProducts;
    const start = (currentPage - 1) * productsPerPage;
    const end = start + productsPerPage;
    
    renderProducts(productsToUse.slice(start, end));
    
    const totalPages = Math.ceil(productsToUse.length / productsPerPage);
    renderPagination(totalPages);
}

document.addEventListener('DOMContentLoaded', () => {
    loadProducts(); 
    setupCategoryFilter();
});

function setupCategoryFilter() {
    const categoryLinks = document.querySelectorAll('.category-group a[data-category]');
    
    categoryLinks.forEach(link => {
        link.addEventListener('click', (e) => {
            e.preventDefault();
            const selectedCategory = link.getAttribute('data-category');
            
            if (selectedCategory === 'all') {
                filteredProducts = null;
            } else {
                filteredProducts = allProducts.filter(product => product.category === selectedCategory);
            }
            
            currentPage = 1;
            updateView(filteredProducts);
        });
    });
}


const minPriceInput = document.getElementById('minPrice');
const maxPriceInput = document.getElementById('maxPrice');

const minPriceDisplay = document.getElementById('minPriceDisplay');
const maxPriceDisplay = document.getElementById('maxPriceDisplay');

minPriceInput.addEventListener('input', () => {
    let val = minPriceInput.value.trim();
    if (val === '' || isNaN(val)) {
        minPriceDisplay.textContent = '0.00';
    } else {
        minPriceDisplay.textContent = Number(val).toFixed(2);
    }
});

maxPriceInput.addEventListener('input', () => {
    let val = maxPriceInput.value.trim();
    if (val === '' || isNaN(val)) {
        maxPriceDisplay.textContent = '0.00';
    } else {
        maxPriceDisplay.textContent = Number(val).toFixed(2);
    }
});
