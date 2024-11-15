package Lab1;

public class App{
    public static volatile boolean ready = false;
    
    public static void main(String[] args){
        Product product = new Product();
        Provider provider = new Provider(product);
        Consumer consumer = new Consumer(product);
        new Thread(provider).start();
        new Thread(consumer).start();
    }
}

class Product{
    public synchronized void provide(){
        if (App.ready)
            return;
        App.ready = true;
        notify();
    }
    public synchronized void consume(){
        while(!App.ready){
            try{
                wait();
            }
            catch(InterruptedException e){ // без этого выдает ошибку. Видимо, обязательно необходима обработка случая прерывания потока (исключения e).
                Thread.currentThread().interrupt();
            }
        }
        App.ready = false;
    }
    public boolean shown = true;
}

class Provider implements Runnable{
    private Product product;
    private boolean doStop = false;

    public Provider(Product product){
        this.product = product;
    }
    public synchronized void doStop(){
        doStop = true;
    }
    private synchronized boolean keepRunning(){
        return doStop == false;
    }

    private void event(){
        product.provide();
    }
    
    public void run(){
        while(keepRunning()){
            try{
                Thread.sleep(1L * 1000L);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
            System.out.println("Инициация события!");
            event();
        }
    }
}
class Consumer implements Runnable{
    private Product product;
    private boolean doStop = false;

    public Consumer(Product product){
        this.product = product;
    }
    public synchronized void doStop(){
        doStop = true;
    }
    private synchronized boolean keepRunning(){
        return doStop == false;
    }

    public void run(){
        while(keepRunning()){
            product.consume();
            System.out.println("Принял событие!");
        }
    }
}