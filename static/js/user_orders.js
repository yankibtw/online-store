document.addEventListener('DOMContentLoaded', async () => {
    const ordersContainer = document.getElementById('ordersContainer');
    if (!ordersContainer) return;

    try {
        const response = await fetch('/api/orders/history', {
            method: 'GET',
            credentials: 'include',
            headers: {
                'Content-Type': 'application/json'
            }
        });

        if (!response.ok) {
            ordersContainer.textContent = 'Не удалось загрузить историю заказов.';
            return;
        }

        const data = await response.json();
        const orders = data.orders;

        if (!orders || orders.length === 0) {
            ordersContainer.textContent = 'История заказов пуста.';
            return;
        }

        let itemsHtml = '';

        orders.forEach(order => {
            let totalPrice = 0;
            let itemsHtmlForOrder = '';

            order.items.forEach(item => {
                const imageUrl = item.image_url || '/static/img/default-image.png';
                const quantity = item.quantity;
                const price = item.price;
                const discountPrice = item.discount_price ?? price;
                const finalPrice = discountPrice * quantity;
                totalPrice += finalPrice;

                itemsHtmlForOrder += `
                <div style="display: flex; gap: 25px; margin-bottom: 15px;">
                    <div class="product" style="width: 100%;">
                        <div style="display: flex; gap: 40px; align-items: center;">
                            <img src="${imageUrl}" alt="${item.name}" style="width: object-fit: cover;" />
                            <div class="product-information">
                                <h2 style="max-width: 600px;">${item.name}</h2>
                                <h5>Размер: ${item.size || 'N/A'}</h5>
                                <h5>Артикул: ${item.sku || 'N/A'}</h5>
                                <h5>Цена за шт: ${discountPrice.toLocaleString('ru-RU')}₽</h5>
                            </div>
                        </div>
                        <div class="product-another-content" style="align-items: flex-end;">
                            <h3 class="product-cost" data-price="${price}">
                                ${finalPrice.toLocaleString('ru-RU')}₽
                                <h5>Количество: ${item.quantity}</h5>
                            </h3>
                        </div>
                    </div>
                </div>
            `;
            });

            itemsHtml += `
                <div style="border: 1px solid #ccc; padding: 15px; margin-bottom: 25px; border-radius: 15px;">
                    <div style="margin-bottom: 45px; margin-left: 12px; margin-top: 10px">
                        <h4 style="font-family: 'Montserrat'">Заказ №${order.order_id}</h4>
                        <h5 style="font-family: 'Montserrat'">Статус: ${order.status}</h5>
                        <h5 style="font-family: 'Montserrat'">Метод оплаты: ${order.payment_method}</h5>
                        <h5 style="font-family: 'Montserrat'">Адрес доставки: ${order.address}</h5>
                        <h5 style="font-family: 'Montserrat'">Дата создания: ${order.created_at.substring(0, 10)}</h5>
                        <h5 style="font-family: 'Montserrat'">Итоговая сумма: ${totalPrice.toLocaleString('ru-RU')}₽</h5>
                    </div>
                    ${itemsHtmlForOrder}
                </div>
            `;
        });

        ordersContainer.innerHTML = itemsHtml;

    } catch (error) {
        console.error('Ошибка при загрузке истории заказов:', error);
        ordersContainer.textContent = 'Произошла ошибка при загрузке истории заказов.';
    }
});
