function selectSize(element) {
        document.querySelectorAll('.size').forEach(size => size.classList.remove('active'));
        element.classList.add('active');
}
